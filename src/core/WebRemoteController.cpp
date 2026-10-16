#include "WebRemoteController.h"
#include "SoundboardProjectController.h"
#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"
#include "SoundTileController.h"

#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QCryptographicHash>
#include <QNetworkInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDebug>

WebRemoteController::WebRemoteController(QObject *parent)
    : QObject(parent)
{
    loadSettings();
    updateCredentialsHash();

    connect(&m_server, &QTcpServer::newConnection, this, &WebRemoteController::onNewConnection);

    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(15000); // 15s keepalive for SSE
    connect(m_heartbeatTimer, &QTimer::timeout, this, &WebRemoteController::onHeartbeat);
}

WebRemoteController::~WebRemoteController()
{
    stopServer();
}

void WebRemoteController::setProjectController(SoundboardProjectController *project)
{
    if (m_projectController == project) return;

    if (m_projectController) {
        disconnect(m_projectController, nullptr, this, nullptr);
        if (m_projectController->tabModel()) {
            disconnect(m_projectController->tabModel(), nullptr, this, nullptr);
        }
    }

    m_projectController = project;
    m_monitoredGrids.clear();
    m_monitoredTiles.clear();

    if (m_projectController) {
        connect(m_projectController, &SoundboardProjectController::projectLoaded,
                this, [this]() { connectTileSignals(); broadcastState(); });
        connect(m_projectController, &SoundboardProjectController::filePathChanged,
                this, [this]() { connectTileSignals(); broadcastState(); });
        connect(m_projectController, &SoundboardProjectController::isDuckingActiveChanged,
                this, &WebRemoteController::broadcastState);
        connect(m_projectController, &SoundboardProjectController::masterVolumeChanged,
                this, &WebRemoteController::broadcastState);
        connect(m_projectController, &SoundboardProjectController::duckingVolumeRatioChanged,
                this, &WebRemoteController::broadcastState);

        if (m_projectController->tabModel()) {
            connect(m_projectController->tabModel(), &SoundboardTabModel::currentIndexChanged,
                    this, &WebRemoteController::broadcastState);
            connect(m_projectController->tabModel(), &SoundboardTabModel::countChanged,
                    this, [this]() { connectTileSignals(); broadcastState(); });
            connect(m_projectController->tabModel(), &SoundboardTabModel::currentGridModelChanged,
                    this, [this]() { connectTileSignals(); broadcastState(); });
            connect(m_projectController->tabModel(), &QAbstractItemModel::dataChanged,
                    this, [this]() { connectTileSignals(); broadcastState(); });
            connect(m_projectController->tabModel(), &QAbstractItemModel::rowsInserted,
                    this, [this]() { connectTileSignals(); broadcastState(); });
            connect(m_projectController->tabModel(), &QAbstractItemModel::rowsRemoved,
                    this, [this]() { connectTileSignals(); broadcastState(); });
        }

        connectTileSignals();
    }
}

void WebRemoteController::connectTileSignals()
{
    if (!m_projectController || !m_projectController->tabModel()) return;

    auto *tabModel = m_projectController->tabModel();
    for (int t = 0; t < tabModel->count(); ++t) {
        auto *grid = tabModel->gridModelAt(t);
        if (!grid) continue;

        if (!m_monitoredGrids.contains(grid)) {
            m_monitoredGrids.insert(grid);
            connect(grid, &QAbstractItemModel::dataChanged, this, [this]() {
                broadcastState();
            });
            connect(grid, &QAbstractItemModel::rowsInserted, this, [this]() {
                connectTileSignals();
                broadcastState();
            });
            connect(grid, &QAbstractItemModel::rowsRemoved, this, [this]() {
                connectTileSignals();
                broadcastState();
            });
            connect(grid, &QAbstractItemModel::modelReset, this, [this]() {
                connectTileSignals();
                broadcastState();
            });
            connect(grid, &QObject::destroyed, this, [this, grid]() {
                m_monitoredGrids.remove(grid);
            });
        }

        for (auto *tile : grid->tiles()) {
            if (!tile || m_monitoredTiles.contains(tile)) continue;
            m_monitoredTiles.insert(tile);

            connect(tile, &SoundTileController::playbackStateChanged, this, &WebRemoteController::broadcastState);
            connect(tile, &SoundTileController::filePathChanged, this, &WebRemoteController::broadcastState);
            connect(tile, &SoundTileController::displayNameChanged, this, &WebRemoteController::broadcastState);
            connect(tile, &SoundTileController::colorChanged, this, &WebRemoteController::broadcastState);
            connect(tile, &SoundTileController::hotkeyChanged, this, &WebRemoteController::broadcastState);
            connect(tile, &SoundTileController::volumeChanged, this, &WebRemoteController::broadcastState);
            connect(tile, &SoundTileController::isLoopedChanged, this, &WebRemoteController::broadcastState);
            connect(tile, &QObject::destroyed, this, [this, tile]() {
                m_monitoredTiles.remove(tile);
            });
        }
    }
}

void WebRemoteController::loadSettings()
{
    QSettings settings("JupiterBroadcasting", "CasterSoundboard");
    m_autoStart = settings.value("web/autoStart", false).toBool();
    m_port = settings.value("web/port", 5052).toInt();
    m_bindAddress = settings.value("web/bindAddress", "0.0.0.0").toString();
    m_username = settings.value("web/username", "caster").toString();
    m_password = settings.value("web/password", "caster").toString();
}

void WebRemoteController::saveSettings()
{
    QSettings settings("JupiterBroadcasting", "CasterSoundboard");
    settings.setValue("web/autoStart", m_autoStart);
    settings.setValue("web/port", m_port);
    settings.setValue("web/bindAddress", m_bindAddress);
    settings.setValue("web/username", m_username);
    settings.setValue("web/password", m_password);
}

void WebRemoteController::updateCredentialsHash()
{
    if (m_username.isEmpty() || m_password.isEmpty() || m_username.contains(':')) {
        m_credentialsHash.clear();
    } else {
        m_credentialsHash = QCryptographicHash::hash((m_username + ":" + m_password).toUtf8(),
                                                     QCryptographicHash::Sha256);
    }
}

void WebRemoteController::setAutoStart(bool autoStart)
{
    if (m_autoStart == autoStart) return;
    m_autoStart = autoStart;
    saveSettings();
    emit autoStartChanged(m_autoStart);
}

void WebRemoteController::setPort(int port)
{
    if (m_port == port || port < 1 || port > 65535) return;
    m_port = port;
    saveSettings();
    emit portChanged(m_port);
    emit statusMessageChanged();
}

void WebRemoteController::setBindAddress(const QString &addr)
{
    if (m_bindAddress == addr) return;
    m_bindAddress = addr;
    saveSettings();
    emit bindAddressChanged(m_bindAddress);
    emit statusMessageChanged();
}

void WebRemoteController::setUsername(const QString &user)
{
    if (m_username == user) return;
    m_username = user;
    saveSettings();
    updateCredentialsHash();
    emit credentialsChanged();
}

void WebRemoteController::setPassword(const QString &pass)
{
    if (m_password == pass) return;
    m_password = pass;
    saveSettings();
    updateCredentialsHash();
    emit credentialsChanged();
}

QString WebRemoteController::localUrl() const
{
    return QString("http://localhost:%1").arg(m_port);
}

QStringList WebRemoteController::availableUrls() const
{
    QStringList urls;
    for (const QString &ip : availableIpAddresses()) {
        urls.append(QString("http://%1:%2").arg(ip).arg(m_port));
    }
    return urls;
}

QStringList WebRemoteController::availableIpAddresses() const
{
    QStringList ips;
    for (const QHostAddress &addr : QNetworkInterface::allAddresses()) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol && !addr.isLoopback()) {
            ips.append(addr.toString());
        }
    }
    if (ips.isEmpty()) {
        ips.append("127.0.0.1");
    }
    return ips;
}

bool WebRemoteController::startServer()
{
    if (m_server.isListening()) {
        return true;
    }

    if (m_credentialsHash.isEmpty()) {
        m_statusMessage = tr("Username and password must not be empty, and username cannot contain ':'");
        emit statusMessageChanged();
        return false;
    }

    QHostAddress addr(m_bindAddress);
    if (m_bindAddress == "0.0.0.0") {
        addr = QHostAddress::AnyIPv4;
    } else if (addr.isNull()) {
        m_statusMessage = tr("Invalid bind address specified: %1").arg(m_bindAddress);
        emit statusMessageChanged();
        return false;
    }

    if (!m_server.listen(addr, static_cast<quint16>(m_port))) {
        m_statusMessage = tr("Failed to bind on %1:%2 - %3").arg(m_bindAddress).arg(m_port).arg(m_server.errorString());
        emit statusMessageChanged();
        emit runningChanged(false);
        return false;
    }

    m_statusMessage = tr("Running on %1:%2").arg(m_bindAddress).arg(m_port);
    m_heartbeatTimer->start();

    connectTileSignals();

    emit statusMessageChanged();
    emit runningChanged(true);
    qInfo() << "[WebRemote] Server listening on" << m_bindAddress << "port" << m_port;
    return true;
}

void WebRemoteController::stopServer()
{
    if (!m_server.isListening()) return;

    m_heartbeatTimer->stop();
    m_server.close();

    const auto activeClients = m_clients;
    m_clients.clear();
    m_streams.clear();

    for (auto *socket : activeClients) {
        if (socket) {
            socket->disconnect(this);
            socket->abort();
            socket->deleteLater();
        }
    }

    m_previousState.clear();
    m_statusMessage = tr("Server stopped");

    emit clientsChanged(0);
    emit statusMessageChanged();
    emit runningChanged(false);
    qInfo() << "[WebRemote] Server stopped.";
}

bool WebRemoteController::toggleServer()
{
    if (isRunning()) {
        stopServer();
        return false;
    }
    return startServer();
}

void WebRemoteController::onNewConnection()
{
    while (auto *socket = m_server.nextPendingConnection()) {
        if (m_clients.size() >= 64) {
            socket->abort();
            socket->deleteLater();
            continue;
        }

        m_clients.insert(socket);
        socket->setReadBufferSize(8192);

        connect(socket, &QTcpSocket::disconnected, this, &WebRemoteController::onSocketDisconnected);
        connect(socket, &QTcpSocket::readyRead, this, &WebRemoteController::onSocketReadyRead);

        // 5-second timeout for non-streaming requests
        QTimer::singleShot(5000, socket, [socket]() {
            if (socket && !socket->property("isStream").toBool()) {
                socket->abort();
            }
        });
    }
}

void WebRemoteController::onSocketDisconnected()
{
    auto *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    m_clients.remove(socket);
    bool wasStream = m_streams.remove(socket);

    socket->deleteLater();

    if (wasStream) {
        emit clientsChanged(m_streams.size());
    }
}

void WebRemoteController::onSocketReadyRead()
{
    auto *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    if (socket->property("handled").toBool()) {
        socket->readAll();
        return;
    }

    QByteArray buf = socket->property("reqBuffer").toByteArray() + socket->readAll();
    if (buf.size() > 8192) {
        sendResponse(socket, "413 Payload Too Large");
        return;
    }

    if (!buf.contains("\r\n\r\n")) {
        socket->setProperty("reqBuffer", buf);
        return;
    }

    socket->setProperty("reqBuffer", QVariant());
    socket->setProperty("handled", true);

    const int headerEnd = buf.indexOf("\r\n\r\n");
    const QByteArray headerBytes = buf.left(headerEnd);
    const QList<QByteArray> lines = headerBytes.split('\n');
    if (lines.isEmpty()) {
        sendResponse(socket, "400 Bad Request");
        return;
    }

    const QList<QByteArray> requestLine = lines.first().trimmed().split(' ');
    if (requestLine.size() != 3 || requestLine[2] != "HTTP/1.1") {
        sendResponse(socket, "400 Bad Request");
        return;
    }

    const QByteArray method = requestLine[0];
    const QByteArray rawPath = requestLine[1];
    const int queryIndex = rawPath.indexOf('?');
    const QByteArray path = (queryIndex >= 0) ? rawPath.left(queryIndex) : rawPath;

    QMap<QByteArray, QByteArray> headers;
    for (int i = 1; i < lines.size(); ++i) {
        const QByteArray line = lines[i].trimmed();
        const int colon = line.indexOf(':');
        if (colon > 0) {
            headers.insert(line.left(colon).trimmed().toLower(), line.mid(colon + 1).trimmed());
        }
    }

    // CORS preflight OPTIONS requests must succeed without credentials
    if (method == "OPTIONS") {
        sendResponse(socket, "204 No Content");
        return;
    }

    // Basic Authentication Check
    const QByteArray authHeader = headers.value("authorization");
    bool authenticated = false;
    if (!authHeader.isEmpty() && authHeader.startsWith("Basic ")) {
        const QByteArray base64Creds = authHeader.mid(6).trimmed();
        const auto decoded = QByteArray::fromBase64Encoding(base64Creds, QByteArray::AbortOnBase64DecodingErrors);
        if (decoded && !m_credentialsHash.isEmpty()) {
            const QByteArray hash = QCryptographicHash::hash(decoded.decoded, QCryptographicHash::Sha256);
            if (hash == m_credentialsHash) {
                authenticated = true;
            }
        }
    }

    if (!authenticated) {
        sendResponse(socket, "401 Unauthorized", "Authentication Required.\n",
                     "text/plain; charset=utf-8",
                     "WWW-Authenticate: Basic realm=\"CasterSoundboard\", charset=\"UTF-8\"\r\n");
        return;
    }

    // Process endpoints
    if (method == "GET" && (path == "/" || path == "/index.html")) {
        if (m_cachedIndexHtml.isEmpty()) {
            QFile file(":/qt/qml/CasterSoundboard/src/web/index.html");
            if (!file.open(QIODevice::ReadOnly)) {
                // Try fallback paths
                QFile fileFallback("src/web/index.html");
                if (fileFallback.open(QIODevice::ReadOnly)) {
                    m_cachedIndexHtml = fileFallback.readAll();
                }
            } else {
                m_cachedIndexHtml = file.readAll();
            }
        }

        if (m_cachedIndexHtml.isEmpty()) {
            sendResponse(socket, "500 Internal Server Error", "index.html not found.");
            return;
        }

        sendResponse(socket, "200 OK", m_cachedIndexHtml, "text/html; charset=utf-8");
    }
    else if (method == "GET" && path == "/api/state") {
        sendResponse(socket, "200 OK", generateStateJson(), "application/json");
    }
    else if (method == "GET" && path == "/events") {
        socket->setProperty("isStream", true);
        m_streams.insert(socket);
        emit clientsChanged(m_streams.size());

        const QByteArray initHeader =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/event-stream\r\n"
            "Cache-Control: no-store, no-cache\r\n"
            "Connection: close\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n";

        socket->write(initHeader);
        const QByteArray state = generateStateJson();
        socket->write("data: " + state + "\n\n");
    }
    else if (method == "POST") {
        // Enforce custom header to prevent CSRF from unauthorized origins
        if (headers.value("x-caster-remote") != "1") {
            sendResponse(socket, "403 Forbidden", "Missing X-Caster-Remote header.");
            return;
        }

        if (path.startsWith("/api/toggle/")) {
            // Path: /api/toggle/<tabIndex>/<tileIndex>
            const auto parts = path.split('/');
            if (parts.size() == 5) {
                int tabIdx = parts[3].toInt();
                int tileIdx = parts[4].toInt();
                if (m_projectController && m_projectController->tabModel()) {
                    auto *grid = m_projectController->tabModel()->gridModelAt(tabIdx);
                    if (grid && tileIdx >= 0 && tileIdx < grid->count()) {
                        auto *tile = grid->tileAt(tileIdx);
                        if (tile) {
                            tile->trigger();
                            sendResponse(socket, "204 No Content");
                            broadcastState();
                            return;
                        }
                    }
                }
            }
            sendResponse(socket, "404 Not Found");
        }
        else if (path.startsWith("/api/play/")) {
            const auto parts = path.split('/');
            if (parts.size() == 5) {
                int tabIdx = parts[3].toInt();
                int tileIdx = parts[4].toInt();
                if (m_projectController && m_projectController->tabModel()) {
                    auto *grid = m_projectController->tabModel()->gridModelAt(tabIdx);
                    if (grid && tileIdx >= 0 && tileIdx < grid->count()) {
                        auto *tile = grid->tileAt(tileIdx);
                        if (tile) {
                            tile->play();
                            sendResponse(socket, "204 No Content");
                            broadcastState();
                            return;
                        }
                    }
                }
            }
            sendResponse(socket, "404 Not Found");
        }
        else if (path.startsWith("/api/stop/")) {
            const auto parts = path.split('/');
            if (parts.size() == 5) {
                int tabIdx = parts[3].toInt();
                int tileIdx = parts[4].toInt();
                if (m_projectController && m_projectController->tabModel()) {
                    auto *grid = m_projectController->tabModel()->gridModelAt(tabIdx);
                    if (grid && tileIdx >= 0 && tileIdx < grid->count()) {
                        auto *tile = grid->tileAt(tileIdx);
                        if (tile) {
                            tile->stop();
                            sendResponse(socket, "204 No Content");
                            broadcastState();
                            return;
                        }
                    }
                }
            }
            sendResponse(socket, "404 Not Found");
        }
        else if (path == "/api/stopAll") {
            if (m_projectController) {
                m_projectController->stopAllSounds();
            }
            sendResponse(socket, "204 No Content");
            broadcastState();
        }
        else if (path == "/api/duck") {
            if (m_projectController) {
                m_projectController->toggleAudioDucking();
            }
            sendResponse(socket, "204 No Content");
            broadcastState();
        }
        else if (path.startsWith("/api/tab/")) {
            const auto parts = path.split('/');
            if (parts.size() == 4) {
                int tabIdx = parts[3].toInt();
                if (m_projectController && m_projectController->tabModel()) {
                    m_projectController->tabModel()->setCurrentIndex(tabIdx);
                    sendResponse(socket, "204 No Content");
                    broadcastState();
                    return;
                }
            }
            sendResponse(socket, "404 Not Found");
        }
        else {
            sendResponse(socket, "404 Not Found");
        }
    }
    else {
        sendResponse(socket, "404 Not Found");
    }
}

void WebRemoteController::sendResponse(QTcpSocket *socket, const QString &status,
                                       const QByteArray &body, const QString &contentType,
                                       const QString &extraHeaders)
{
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) return;

    QString defaultHeaders = "Access-Control-Allow-Origin: *\r\n"
                             "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                             "Access-Control-Allow-Headers: Authorization, Content-Type, X-Caster-Remote\r\n"
                             "Cache-Control: no-store, no-cache, must-revalidate\r\n";
    if (!extraHeaders.isEmpty()) {
        defaultHeaders += extraHeaders;
    }

    QByteArray resp = QString("HTTP/1.1 %1\r\n"
                              "Content-Type: %2\r\n"
                              "Content-Length: %3\r\n"
                              "Connection: close\r\n"
                              "%4\r\n")
                          .arg(status)
                          .arg(contentType)
                          .arg(body.size())
                          .arg(defaultHeaders)
                          .toUtf8();
    resp.append(body);
    socket->write(resp);
    socket->disconnectFromHost();
}

QByteArray WebRemoteController::generateStateJson() const
{
    QJsonObject root;
    root["appName"] = "CasterSoundboard";
    root["version"] = "2.0.0";

    if (!m_projectController) {
        return QJsonDocument(root).toJson(QJsonDocument::Compact);
    }

    root["isDuckingActive"] = m_projectController->isDuckingActive();
    root["duckingVolumeRatio"] = m_projectController->duckingVolumeRatio();
    root["masterVolume"] = m_projectController->masterVolume();

    auto *tabModel = m_projectController->tabModel();
    if (tabModel) {
        root["activeTabIndex"] = tabModel->currentIndex();
        QJsonArray tabsArr;

        for (int t = 0; t < tabModel->count(); ++t) {
            QJsonObject tabObj;
            tabObj["index"] = t;
            tabObj["id"] = tabModel->tabIdAt(t);
            tabObj["name"] = tabModel->tabNameAt(t);
            tabObj["hotkey"] = tabModel->tabHotkeyAt(t);

            QJsonArray tilesArr;
            auto *grid = tabModel->gridModelAt(t);
            if (grid) {
                for (int i = 0; i < grid->count(); ++i) {
                    auto *tile = grid->tileAt(i);
                    if (!tile) continue;

                    QJsonObject tileObj;
                    tileObj["index"] = i;
                    tileObj["id"] = tile->id();
                    tileObj["name"] = tile->displayName();
                    tileObj["color"] = tile->color();
                    tileObj["hotkey"] = tile->hotkey();
                    tileObj["hasFile"] = !tile->filePath().isEmpty();
                    tileObj["isLooped"] = tile->isLooped();
                    tileObj["volume"] = tile->volume();

                    QString stateStr = "Stopped";
                    if (tile->playbackState() == QMediaPlayer::PlayingState) {
                        stateStr = "Playing";
                    } else if (tile->playbackState() == QMediaPlayer::PausedState) {
                        stateStr = "Paused";
                    }
                    tileObj["state"] = stateStr;
                    tileObj["positionMs"] = static_cast<double>(tile->positionMs());
                    tileObj["durationMs"] = static_cast<double>(tile->durationMs());

                    tilesArr.append(tileObj);
                }
            }
            tabObj["tiles"] = tilesArr;
            tabsArr.append(tabObj);
        }
        root["tabs"] = tabsArr;
    }

    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

void WebRemoteController::broadcastState()
{
    if (m_streams.isEmpty()) return;

    const QByteArray state = generateStateJson();
    if (state == m_previousState) return;
    m_previousState = state;

    const QByteArray msg = "data: " + state + "\n\n";
    const auto active = m_streams;

    for (auto *socket : active) {
        if (!socket || socket->state() != QAbstractSocket::ConnectedState) continue;
        if (socket->bytesToWrite() > 256 * 1024) {
            socket->abort();
            continue;
        }
        socket->write(msg);
    }
}

void WebRemoteController::onHeartbeat()
{
    if (m_streams.isEmpty()) return;

    const QByteArray ping = ": keepalive\n\n";
    const auto active = m_streams;

    for (auto *socket : active) {
        if (!socket || socket->state() != QAbstractSocket::ConnectedState) continue;
        if (socket->bytesToWrite() > 256 * 1024) {
            socket->abort();
            continue;
        }
        socket->write(ping);
    }
}
