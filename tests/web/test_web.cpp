#include "web/WebRemote.h"
#include "MainWindow.h"
#include <QSettings>
#include <QTemporaryDir>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QTimer>
#include "CasterBoard.h"
#include "CasterPlayer.h"
#include <QApplication>
#include <QTabWidget>
#include <QTcpSocket>
#include <QLabel>
#include <QGridLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTest>
#include <QTemporaryFile>
#include <QDataStream>
#include <functional>
#include <cstdlib>

static void check(bool ok, const char *message) {
    if (!ok) { qCritical("FAIL: %s", message); std::exit(1); }
}
static bool until(std::function<bool()> predicate) {
    for (int i=0; i<300; ++i) { if (predicate()) return true; QTest::qWait(10); }
    return predicate();
}
static const QString testUser = QStringLiteral("caster");
static const QString testPassword = QStringLiteral("test:pässword");
static QByteArray authorization() {
    return "Authorization: Basic " + (testUser + ':' + testPassword).toUtf8().toBase64() + "\r\n";
}
static QByteArray request(const QByteArray &path, const QByteArray &method="GET", const QByteArray &headers={}, bool authenticate=true) {
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, 15052);
    check(until([&]{return socket.state()==QTcpSocket::ConnectedState;}), "connect");
    const QByteArray bytes=method+" "+path+" HTTP/1.1\r\nHost: localhost:15052\r\n"+headers+(authenticate ? authorization() : QByteArray())+"\r\n";
    // Exercise request fragmentation.
    socket.write(bytes.left(8)); QTest::qWait(10); socket.write(bytes.mid(8));
    check(until([&]{return socket.state()==QTcpSocket::UnconnectedState;}), "HTTP response completes");
    return socket.readAll();
}
static QJsonArray boards() {
    auto response=request("/api/state");
    check(response.startsWith("HTTP/1.1 200"), "state status");
    return QJsonDocument::fromJson(response.mid(response.indexOf("\r\n\r\n")+4)).object()["boards"].toArray();
}
int main(int argc, char **argv) {
    QApplication app(argc,argv);
    QTemporaryDir settingsDir;
    check(settingsDir.isValid(), "isolated settings directory");
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settingsDir.path());
    QSettings settings("CasterSoundboard", "CasterSoundboard");
    settings.clear();
    QTabWidget tabs;
    WebRemote remote(&tabs);
    check(!remote.start(15052, QHostAddress::AnyIPv4, "", ""), "missing credentials prevent startup");
    check(!remote.start(15052, QHostAddress::AnyIPv4, "bad:user", testPassword), "colon in username rejected");
    check(remote.start(15052, QHostAddress::AnyIPv4, testUser, testPassword), "listen");
    for (const QByteArray &path : {QByteArray("/"), QByteArray("/api/state"), QByteArray("/events"), QByteArray("/api/toggle/unknown/key")}) {
        const auto response = request(path, path.startsWith("/api/toggle") ? "POST" : "GET", {}, false);
        check(response.startsWith("HTTP/1.1 401"), "all routes require authentication");
        check(response.contains("WWW-Authenticate: Basic realm=\"CasterSoundboard\", charset=\"UTF-8\""), "browser auth challenge");
    }
    check(request("/", "GET", "Authorization: Basic " + QByteArray("caster:wrong").toBase64() + "\r\n", false)
          .startsWith("HTTP/1.1 401"), "wrong password rejected");
    check(request("/", "GET", "Authorization: Basic !!!\r\n", false).startsWith("HTTP/1.1 401"), "invalid base64 rejected");
    WebRemote conflicting(&tabs);
    check(!conflicting.start(15052, QHostAddress::AnyIPv4, testUser, testPassword), "port conflict reported");
    check(boards().isEmpty(), "empty startup");
    check(request("/").contains("new EventSource('/events')"), "embedded page");
    check(request("/missing").startsWith("HTTP/1.1 404"), "unknown route");
    auto first=new CasterBoard; auto second=new CasterBoard;
    tabs.addTab(first,"First"); tabs.addTab(second,"Second");
    auto player=second->players->value(",");
    // A silent WAV exercises real QMediaPlayer state without audible test output.
    QTemporaryFile wav("/tmp/caster-web-XXXXXX.wav"); check(wav.open(), "fixture");
    QDataStream out(&wav);out.setByteOrder(QDataStream::LittleEndian);
    wav.write("RIFF",4);out << quint32(32036);wav.write("WAVEfmt ",8);
    out << quint32(16) << quint16(1) << quint16(1) << quint32(8000) << quint32(16000) << quint16(2) << quint16(16);
    wav.write("data",4);out << quint32(32000);wav.write(QByteArray(32000,0));wav.flush();
    *player->soundFilePath=wav.fileName();
    player->player->setMedia(QUrl::fromLocalFile(wav.fileName()));
    check(until([&]{return player->player->mediaStatus()==QMediaPlayer::LoadedMedia;}), "WAV loaded");
    player->soundNameLabel->setText("<Sound> & café");
    player->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #123456,stop:1 #123456);");
    auto state=boards();check(state.size()==2, "all tabs exposed");
    check(state[0].toObject()["players"].toArray().isEmpty(), "empty slots hidden");
    check(state[1].toObject()["players"].toArray()[0].toObject()["name"]=="<Sound> & café", "display label preserved");
    auto metadata=state[1].toObject()["players"].toArray()[0].toObject();
    check(metadata["color"]=="#123456", "applied colour exposed");
    check(metadata["row"].toInt()==3 && metadata["column"].toInt()==7, "comma slot position");
    check(state[1].toObject()["rows"].toInt()==4 && state[1].toObject()["columns"].toInt()==8, "full grid dimensions");
    if (app.arguments().contains("--preview")) return app.exec();
    QByteArray path="/api/toggle/"+state[1].toObject()["id"].toString().toUtf8()+"/%2C";
    QTcpSocket stream;stream.connectToHost(QHostAddress::LocalHost,15052);
    check(until([&]{return stream.state()==QTcpSocket::ConnectedState;}), "stream connect");
    stream.write("GET /events HTTP/1.1\r\nHost: localhost:15052\r\n" + authorization() + "\r\n");
    check(until([&]{return stream.bytesAvailable()>0;}), "initial stream");stream.readAll();
    player->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #FFDDAA,stop:1 #FFDDAA);");
    auto grid=qobject_cast<QGridLayout *>(player->parentWidget()->layout());
    check(grid, "player layout");
    grid->removeWidget(player);grid->addWidget(player,4,2);
    QByteArray metadataEvents;
    check(until([&]{metadataEvents+=stream.readAll();return metadataEvents.contains("#FFDDAA") && metadataEvents.contains("\"row\":4");}), "colour and layout updates streamed");
    metadata=boards()[1].toObject()["players"].toArray()[0].toObject();
    check(metadata["row"].toInt()==4 && metadata["column"].toInt()==2, "position follows layout rather than key order");
    check(request(path, "POST", "X-Caster-Remote: 1\r\n", false).startsWith("HTTP/1.1 401"), "unauthenticated playback rejected");
    check(player->player->state() == QMediaPlayer::StoppedState, "unauthenticated request does not start playback");
    check(request(path,"POST").startsWith("HTTP/1.1 403"), "reject unguarded mutation");
    check(request(path,"POST","X-Caster-Remote: 1\r\nOrigin: http://other.invalid\r\n").startsWith("HTTP/1.1 403"), "reject cross origin");
    check(request(path,"POST","X-Caster-Remote: 1\r\n").startsWith("HTTP/1.1 204"), "toggle comma key");
    check(player->player->state()==QMediaPlayer::PlayingState, "real playback starts");
    QByteArray events;
    check(until([&]{events+=stream.readAll();return events.contains("Playing");}), "stream playing feedback");
    events.clear();
    check(until([&]{events+=stream.readAll();return events.contains("Stopped");}), "stream natural completion");
    tabs.removeTab(0);delete first;
    check(request(path,"POST","X-Caster-Remote: 1\r\n").startsWith("HTTP/1.1 204"), "identity survives tab removal");
    player->stopSound();events.clear();
    check(until([&]{events+=stream.readAll();return events.contains("Stopped");}), "desktop stop feedback");
    tabs.removeTab(0);delete second;
    check(request(path,"POST","X-Caster-Remote: 1\r\n").startsWith("HTTP/1.1 404"), "closed tab rejected");
    check(request("/","GET","Content-Length: 99\r\n").startsWith("HTTP/1.1 400"), "bodies rejected");
    check(request("/","GET","Long: "+QByteArray(9000,'x')+"\r\n").startsWith("HTTP/1.1 413"), "oversized headers rejected");
    remote.stop();
    check(!remote.isRunning(), "stop closes listener");
    check(until([&]{ return stream.state() == QTcpSocket::UnconnectedState; }), "stop disconnects SSE clients");
    check(remote.start(15052, QHostAddress::LocalHost, testUser, testPassword), "restart on specific IP");
    check(boards().isEmpty(), "HTTP works after restart");
    remote.stop();
    check(remote.start(15052, QHostAddress::LocalHost, testUser, "replacement"), "restart with changed credentials");
    check(request("/").startsWith("HTTP/1.1 401"), "old credentials rejected after restart");
    check(request("/", "GET", "Authorization: Basic " + QByteArray("caster:replacement").toBase64() + "\r\n", false)
          .startsWith("HTTP/1.1 200"), "new credentials accepted after restart");
    remote.stop();

    {
        MainWindow window;
        auto webStatus = window.findChild<QPushButton *>("webServerStatus");
        auto oscStatus = window.findChild<QPushButton *>("oscServerStatus");
        auto button = window.findChild<QPushButton *>("webServerButton");
        window.show();
        QApplication::processEvents();
        check(!webStatus->isVisible() && !oscStatus->isVisible(), "indicators hidden without tabs");
        auto windowTabs = window.findChild<QTabWidget *>();
        auto emptyTab = new QWidget;
        windowTabs->addTab(emptyTab, "Layout check");
        QApplication::processEvents();
        check(webStatus->isVisible() && oscStatus->isVisible(), "indicators visible with a tab");
        check(webStatus->geometry().right() < oscStatus->geometry().left()
              && webStatus->geometry().top() == oscStatus->geometry().top(), "indicators side by side");
        windowTabs->removeTab(0);
        delete emptyTab;
        check(!webStatus->isVisible() && !oscStatus->isVisible(), "indicators hidden after last tab closes");
        check(button && button->isEnabled(), "web toolbar enabled");
        check(webStatus && webStatus->accessibleName() == "Web Server: Off", "web defaults off");
        check(oscStatus && oscStatus->accessibleName() == "OSC: Off", "OSC defaults off");
        check(window.startOSCServer(15053), "OSC start");
        check(oscStatus->accessibleName() == "OSC: On", "OSC indicator on");
        oscStatus->click();
        check(!window.isOSCServerRunning() && !oscStatus->isChecked(), "OSC button stops server");
        oscStatus->click();
        check(window.isOSCServerRunning() && oscStatus->isChecked(), "OSC button starts server without success dialog");
        oscStatus->click();
        check(oscStatus->accessibleName() == "OSC: Off", "OSC indicator off");
        QTimer::singleShot(0, [&] {
            auto dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            check(dlg, "web settings opened");
            auto ip = dlg->findChild<QLineEdit *>("webListenIp");
            auto port = dlg->findChild<QSpinBox *>("webListenPort");
            auto toggle = dlg->findChild<QPushButton *>("webServerToggle");
            check(ip->text() == "0.0.0.0", "all interfaces default");
            port->setValue(15052);
            auto user = dlg->findChild<QLineEdit *>("webUser");
            auto password = dlg->findChild<QLineEdit *>("webPassword");
            check(user && password && password->echoMode() == QLineEdit::Normal, "visible User and Password fields");
            user->setText(testUser);
            password->setText(testPassword);
            ip->setText("invalid");
            toggle->click();
            check(webStatus->accessibleName() == "Web Server: Off", "invalid address rejected");
            ip->setText("127.0.0.1");
            check(conflicting.start(15052, QHostAddress::AnyIPv4, testUser, testPassword), "occupy configured port");
            toggle->click();
            check(webStatus->accessibleName() == "Web Server: Off", "failed start stays off");
            conflicting.stop();
            toggle->click();
            check(webStatus->accessibleName() == "Web Server: On", "web indicator on");
            check(ip->isReadOnly() && !port->isEnabled(), "running config locked");
            toggle->click();
            check(webStatus->accessibleName() == "Web Server: Off", "web indicator off");
            check(!ip->isReadOnly() && port->isEnabled(), "stopped config editable");
            toggle->click();
            for (auto exit : dlg->findChildren<QPushButton *>())
                if (exit->text() == "Exit") exit->click();
        });
        button->click();
        check(webStatus->accessibleName() == "Web Server: On", "Exit leaves server running");
        check(settings.value("web/listenIp").toString() == "127.0.0.1", "IP saved");
        check(settings.value("web/port").toInt() == 15052, "port saved");
        check(settings.value("web/user").toString() == testUser && settings.value("web/password").toString() == testPassword,
              "credentials saved");
        webStatus->click();
        check(!webStatus->isChecked(), "web button stops server");
        check(conflicting.start(15052, QHostAddress::AnyIPv4, testUser, testPassword), "stopped button releases port");
        QTimer::singleShot(0, [&] {
            auto failure = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            check(failure && !webStatus->isChecked(), "failed toggle shows error and stays off");
            failure->accept();
        });
        webStatus->click();
        conflicting.stop();
        webStatus->click();
        check(webStatus->isChecked() && boards().isEmpty(), "web button starts saved endpoint");
        QTimer::singleShot(0, [&] {
            auto dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            auto checkbox = dlg->findChild<QCheckBox *>("autoStartWebCheckbox");
            check(checkbox && !checkbox->isChecked(), "startup preference defaults off");
            checkbox->setChecked(true);
            dlg->accept();
        });
        window.openPreferences();
        check(settings.value("web/autoStart").toBool(), "startup preference saved");
    }
    {
        MainWindow restarted;
        check(restarted.findChild<QPushButton *>("webServerStatus")->accessibleName() == "Web Server: On",
              "startup uses saved endpoint and enabled preference");
        check(boards().isEmpty(), "saved port serves authenticated requests");
        check(request("/", "GET", {}, false).startsWith("HTTP/1.1 401"), "startup remains protected");
    }
    qInfo("PASS: web remote integration tests");
}
