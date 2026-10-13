#include "WebRemote.h"
#include "../CasterBoard.h"
#include "../CasterPlayer.h"
#include <QFile>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QTabWidget>
#include <QTcpSocket>
#include <QTimer>
#include <QUuid>
#include <QUrl>
#include <QVariant>

namespace {
QString boardId(CasterBoard *board)
{
    if (!board->property("webId").isValid())
        board->setProperty("webId", QUuid::createUuid().toString(QUuid::WithoutBraces));
    return board->property("webId").toString();
}
void reply(QTcpSocket *socket, const QByteArray &status, const QByteArray &body = {},
           const QByteArray &type = "text/plain; charset=utf-8", const QByteArray &extraHeaders = {})
{
    socket->setProperty("handled", true);
    socket->write("HTTP/1.1 " + status + "\r\nContent-Type: " + type +
                  "\r\nCache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\n"
                  "X-Frame-Options: DENY\r\n" + extraHeaders + "Connection: close\r\nContent-Length: " +
                  QByteArray::number(body.size()) + "\r\n\r\n" + body);
    socket->disconnectFromHost();
}
}

WebRemote::WebRemote(QTabWidget *tabs, QObject *parent)
    : QObject(parent), tabs(tabs), server(this)
{
    connect(&server, &QTcpServer::newConnection, this, &WebRemote::acceptConnections);
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &WebRemote::broadcast);
    timer->start(100);
}
WebRemote::~WebRemote()
{
    stop();
}
bool WebRemote::start(quint16 port, const QHostAddress &address, const QString &user, const QString &password)
{
    if (isRunning()) {
        startError = tr("Web Server is already running.");
        return false;
    }
    if (user.isEmpty() || password.isEmpty() || user.contains(':')) {
        startError = tr("Set User and Password in Web Server settings before starting. User cannot contain a colon.");
        emit runningChanged(false);
        return false;
    }
    credentialsHash = QCryptographicHash::hash((user + ':' + password).toUtf8(), QCryptographicHash::Sha256);
    const bool started = server.listen(address, port);
    startError = started ? QString() : server.errorString();
    if (!started) credentialsHash.clear();
    emit runningChanged(started);
    return started;
}
void WebRemote::stop()
{
    server.close();
    // Disconnect callbacks before aborting, as abort can emit disconnected inline.
    const auto active = clients;
    clients.clear();
    streams.clear();
    for (auto socket : active) {
        socket->disconnect(this);
        socket->abort();
        socket->deleteLater();
    }
    credentialsHash.clear();
    previous.clear();
    ticks = 0;
    emit runningChanged(false);
}
QByteArray WebRemote::snapshot()
{
    QJsonArray boards;
    for (int i = 0; i < tabs->count(); ++i) {
        auto board = qobject_cast<CasterBoard *>(tabs->widget(i));
        if (!board) continue;
        QJsonArray players;
        int rows = 0, columns = 0;
        for (const QString &key : *board->int_to_player_key) {
            const QPoint position = board->playerGridPosition(key);
            if (position.x() < 0 || position.y() < 0) continue;
            rows = qMax(rows, position.y() + 1);
            columns = qMax(columns, position.x() + 1);
            auto p = board->players->value(key);
            if (!p || p->soundFilePath->isEmpty()) continue;
            if (!watchedPlayers.contains(p->player)) {
                auto media = p->player;
                watchedPlayers.insert(media);
                connect(media, &QMediaPlayer::playbackStateChanged, this, [this] { broadcast(); });
                connect(media, &QObject::destroyed, this, [this, media] { watchedPlayers.remove(media); });
            }
            const QString state = p->player->state() == QMediaPlayer::PlayingState ? "Playing" :
                                  p->player->state() == QMediaPlayer::PausedState ? "Paused" : "Stopped";
            players.append(QJsonObject{{"key", key}, {"name", p->soundNameLabel->text()},
                                       {"color", p->displayColor()}, {"row", position.y()}, {"column", position.x()},
                                       {"state", state}, {"error", p->player->errorString()}});
        }
        boards.append(QJsonObject{{"id", boardId(board)}, {"name", tabs->tabText(i)},
                                  {"rows", rows}, {"columns", columns}, {"players", players}});
    }
    return QJsonDocument(QJsonObject{{"boards", boards}}).toJson(QJsonDocument::Compact);
}
void WebRemote::acceptConnections()
{
    while (auto socket = server.nextPendingConnection()) {
        if (clients.size() >= 64) { socket->abort(); socket->deleteLater(); continue; }
        clients.insert(socket);
        socket->setReadBufferSize(8193);
        connect(socket, &QTcpSocket::disconnected, this, [this, socket] {
            clients.remove(socket); streams.remove(socket); socket->deleteLater();
        });
        connect(socket, &QTcpSocket::readyRead, this, [this, socket] { readRequest(socket); });
        QTimer::singleShot(5000, socket, [socket] {
            if (!socket->property("stream").toBool()) socket->abort();
        });
    }
}
void WebRemote::readRequest(QTcpSocket *socket)
{
    if (socket->property("handled").toBool()) { socket->readAll(); return; }
    QByteArray request = socket->property("request").toByteArray() + socket->readAll();
    if (request.size() > 8192) { reply(socket, "413 Payload Too Large"); return; }
    if (!request.contains("\r\n\r\n")) { socket->setProperty("request", request); return; }
    socket->setProperty("request", QVariant());
    const auto lines = request.left(request.indexOf("\r\n\r\n")).split('\n');
    const auto first = lines.first().trimmed().split(' ');
    if (first.size() != 3 || first[2] != "HTTP/1.1") { reply(socket, "400 Bad Request"); return; }
    QMap<QByteArray, QByteArray> headers;
    for (int i = 1; i < lines.size(); ++i) {
        int colon = lines[i].indexOf(':');
        auto name = lines[i].left(colon).trimmed().toLower();
        if (colon <= 0 || headers.contains(name)) { reply(socket, "400 Bad Request"); return; }
        headers.insert(name, lines[i].mid(colon + 1).trimmed());
    }
    if (headers.value("host").isEmpty() || headers.contains("transfer-encoding") ||
        (headers.contains("content-length") && headers.value("content-length") != "0")) {
        reply(socket, "400 Bad Request"); return;
    }
    const auto authorization = headers.value("authorization");
    const auto separator = authorization.indexOf(' ');
    const auto decoded = QByteArray::fromBase64Encoding(authorization.mid(separator + 1).trimmed(),
                                                       QByteArray::AbortOnBase64DecodingErrors);
    if (separator < 0 || authorization.left(separator).compare("Basic", Qt::CaseInsensitive) != 0 ||
        !decoded || credentialsHash.isEmpty() ||
        QCryptographicHash::hash(decoded.decoded, QCryptographicHash::Sha256) != credentialsHash) {
        reply(socket, "401 Unauthorized", "Authentication required.\n", "text/plain; charset=utf-8",
              "WWW-Authenticate: Basic realm=\"CasterSoundboard\", charset=\"UTF-8\"\r\n");
        return;
    }
    const auto method = first[0];
    const auto path = first[1];
    if (method == "GET" && path == "/") {
        QFile file(":/web/index.html");
        if (!file.open(QIODevice::ReadOnly)) { reply(socket, "500 Internal Server Error"); return; }
        reply(socket, "200 OK", file.readAll(), "text/html; charset=utf-8");
    } else if (method == "GET" && path == "/api/state") {
        reply(socket, "200 OK", snapshot(), "application/json");
    } else if (method == "GET" && path == "/events") {
        socket->setProperty("handled", true);
        socket->setProperty("stream", true);
        socket->write("HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n");
        streams.insert(socket);
        socket->write("data: " + snapshot() + "\n\n");
    } else if (method == "POST" && path.startsWith("/api/toggle/")) {
        // A custom header forces cross-origin browser requests through a rejected CORS preflight.
        if (headers.value("x-caster-remote") != "1" ||
            (headers.contains("origin") && headers.value("origin") != "http://" + headers.value("host"))) {
            reply(socket, "403 Forbidden"); return;
        }
        const auto parts = path.split('/');
        if (parts.size() != 5) { reply(socket, "400 Bad Request"); return; }
        for (int i = 0; i < tabs->count(); ++i) {
            auto board = qobject_cast<CasterBoard *>(tabs->widget(i));
            if (!board || boardId(board).toUtf8() != parts[3]) continue;
            auto player = board->players->value(QUrl::fromPercentEncoding(parts[4]), nullptr);
            if (!player || player->soundFilePath->isEmpty()) break;
            player->play_stop_toggle();
            reply(socket, "204 No Content");
            broadcast();
            return;
        }
        reply(socket, "404 Not Found");
    } else {
        reply(socket, "404 Not Found");
    }
}
void WebRemote::broadcast()
{
    if (streams.isEmpty()) return;
    const auto state = snapshot();
    const bool changed = state != previous;
    previous = state;
    const bool heartbeat = ++ticks % 150 == 0;
    const auto active = streams;
    for (auto socket : active) {
        if (socket->bytesToWrite() > 256 * 1024) { socket->abort(); continue; }
        if (changed) socket->write("data: " + state + "\n\n");
        else if (heartbeat) socket->write(": keepalive\n\n");
    }
}
