#include <QTest>
#include <QSignalSpy>
#include <QTcpSocket>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "WebRemoteController.h"
#include "SoundboardProjectController.h"
#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"
#include "SoundTileController.h"

class TestWebRemote : public QObject
{
    Q_OBJECT

private:
    static QByteArray sendHttpRequest(quint16 port, const QString &method, const QString &path,
                                      const QMap<QString, QString> &headers = QMap<QString, QString>())
    {
        QTcpSocket socket;
        socket.connectToHost("127.0.0.1", port);
        for (int i = 0; i < 200 && socket.state() != QTcpSocket::ConnectedState; ++i) {
            QTest::qWait(10);
        }
        if (socket.state() != QTcpSocket::ConnectedState) {
            return QByteArray();
        }

        QByteArray req = method.toUtf8() + " " + path.toUtf8() + " HTTP/1.1\r\n"
                         "Host: 127.0.0.1:" + QByteArray::number(port) + "\r\n";

        for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
            req += it.key().toUtf8() + ": " + it.value().toUtf8() + "\r\n";
        }
        req += "\r\n";

        socket.write(req);

        QByteArray response;
        for (int i = 0; i < 200; ++i) {
            QTest::qWait(10);
            if (socket.bytesAvailable() > 0) {
                response += socket.readAll();
            }
            if (socket.state() == QTcpSocket::UnconnectedState && !response.isEmpty()) {
                break;
            }
        }

        return response;
    }

private slots:
    void initTestCase()
    {
        QSettings s("JupiterBroadcasting", "CasterSoundboard");
        s.setValue("web/port", 15052);
        s.setValue("web/bindAddress", "127.0.0.1");
        s.setValue("web/username", "testuser");
        s.setValue("web/password", "testpass");
        s.setValue("web/autoStart", false);
    }

    void cleanupTestCase()
    {
        QSettings s("JupiterBroadcasting", "CasterSoundboard");
        s.remove("web");
    }

    void testInitialState()
    {
        WebRemoteController web;
        QCOMPARE(web.isRunning(), false);
        QCOMPARE(web.port(), 15052);
        QCOMPARE(web.bindAddress(), QString("127.0.0.1"));
        QCOMPARE(web.username(), QString("testuser"));
        QCOMPARE(web.password(), QString("testpass"));
        QCOMPARE(web.autoStart(), false);
        QCOMPARE(web.connectedClients(), 0);
    }

    void testServerStartStop()
    {
        WebRemoteController web;
        QCOMPARE(web.startServer(), true);
        QCOMPARE(web.isRunning(), true);
        QCOMPARE(web.localUrl(), QString("http://localhost:15052"));

        web.stopServer();
        QCOMPARE(web.isRunning(), false);
    }

    void testAuthentication()
    {
        SoundboardProjectController project;
        WebRemoteController web;
        web.setProjectController(&project);
        QVERIFY(web.startServer());

        // 1. Request without auth header should be rejected with 401
        QByteArray resp1 = sendHttpRequest(15052, "GET", "/api/state");
        QVERIFY(resp1.startsWith("HTTP/1.1 401 Unauthorized"));
        QVERIFY(resp1.contains("WWW-Authenticate: Basic"));

        // 2. Request with wrong auth credentials should be rejected with 401
        QMap<QString, QString> badHeaders;
        badHeaders["Authorization"] = "Basic " + QByteArray("testuser:wrongpass").toBase64();
        QByteArray resp2 = sendHttpRequest(15052, "GET", "/api/state", badHeaders);
        QVERIFY(resp2.startsWith("HTTP/1.1 401 Unauthorized"));

        // 3. Request with correct credentials should return 200 OK and JSON
        QMap<QString, QString> goodHeaders;
        goodHeaders["Authorization"] = "Basic " + QByteArray("testuser:testpass").toBase64();
        QByteArray resp3 = sendHttpRequest(15052, "GET", "/api/state", goodHeaders);
        QVERIFY(resp3.startsWith("HTTP/1.1 200 OK"));
        QVERIFY(resp3.contains("application/json"));
        QVERIFY(resp3.contains("\"appName\":\"CasterSoundboard\""));

        web.stopServer();
    }

    void testStopAllAndDucking()
    {
        SoundboardProjectController project;
        WebRemoteController web;
        web.setProjectController(&project);
        QVERIFY(web.startServer());

        QMap<QString, QString> authHeaders;
        authHeaders["Authorization"] = "Basic " + QByteArray("testuser:testpass").toBase64();

        // 1. POST without X-Caster-Remote header should be rejected with 403 Forbidden
        QByteArray respNoCsrf = sendHttpRequest(15052, "POST", "/api/duck", authHeaders);
        QVERIFY(respNoCsrf.startsWith("HTTP/1.1 403 Forbidden"));

        // 2. POST with X-Caster-Remote header should toggle ducking
        QMap<QString, QString> validHeaders = authHeaders;
        validHeaders["X-Caster-Remote"] = "1";

        QCOMPARE(project.isDuckingActive(), false);
        QByteArray respDuck = sendHttpRequest(15052, "POST", "/api/duck", validHeaders);
        QVERIFY(respDuck.startsWith("HTTP/1.1 204 No Content"));
        QCOMPARE(project.isDuckingActive(), true);

        // 3. POST stopAll
        QByteArray respStop = sendHttpRequest(15052, "POST", "/api/stopAll", validHeaders);
        QVERIFY(respStop.startsWith("HTTP/1.1 204 No Content"));

        web.stopServer();
    }
};

QTEST_MAIN(TestWebRemote)
#include "test_web_remote.moc"
