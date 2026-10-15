#include <QTest>
#include <QSignalSpy>
#include <QUdpSocket>
#include <QSettings>
#include <QtEndian>
#include <cstring>

#include "OscServerController.h"
#include "SoundboardProjectController.h"
#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"
#include "SoundTileController.h"

class TestOscServer : public QObject
{
    Q_OBJECT

private:
    static QByteArray makeOscMessage(const QString &address, const QString &typeTags, const QByteArray &args = QByteArray())
    {
        QByteArray packet;
        QByteArray addr = address.toUtf8();
        addr.append('\0');
        while ((addr.size() % 4) != 0) addr.append('\0');
        packet.append(addr);

        QByteArray tags = ("," + typeTags).toUtf8();
        tags.append('\0');
        while ((tags.size() % 4) != 0) tags.append('\0');
        packet.append(tags);

        packet.append(args);
        return packet;
    }

private slots:
    void initTestCase()
    {
        QSettings s("CasterSoundboard", "OscSettings");
        s.clear();
    }

    void cleanupTestCase()
    {
        QSettings s("CasterSoundboard", "OscSettings");
        s.clear();
    }

    void testInitialState()
    {
        OscServerController osc;
        QCOMPARE(osc.isRunning(), false);
        QCOMPARE(osc.inboundPort(), 5051);
        QCOMPARE(osc.outboundHost(), QString("127.0.0.1"));
        QCOMPARE(osc.outboundPort(), 9000);
        QCOMPARE(osc.packetsReceived(), 0);
    }

    void testServerStartStop()
    {
        OscServerController osc;
        osc.setInboundPort(58991); // Pick non-standard port for testing
        QVERIFY(osc.startServer());
        QCOMPARE(osc.isRunning(), true);

        osc.stopServer();
        QCOMPARE(osc.isRunning(), false);
    }

    void testReceiveCommandStopAll()
    {
        SoundboardProjectController project;
        OscServerController osc;
        osc.setProjectController(&project);
        osc.setInboundPort(58992);
        QVERIFY(osc.startServer());

        QUdpSocket client;
        QByteArray packet = makeOscMessage("/stop_all", "");
        client.writeDatagram(packet, QHostAddress::LocalHost, 58992);

        QTRY_COMPARE(osc.packetsReceived(), 1);
        QCOMPARE(osc.lastMessageAddress(), QString("/stop_all"));

        osc.stopServer();
    }

    void testReceiveCommandDucking()
    {
        SoundboardProjectController project;
        OscServerController osc;
        osc.setProjectController(&project);
        osc.setInboundPort(58993);
        QVERIFY(osc.startServer());

        QCOMPARE(project.isDuckingActive(), false);

        // Send ducking ON: /glo/m/audio_d_s with int 1
        QUdpSocket client;
        qint32 valOn = qToBigEndian<qint32>(1);
        QByteArray argOn(reinterpret_cast<const char*>(&valOn), sizeof(valOn));
        QByteArray packetOn = makeOscMessage("/glo/m/audio_d_s", "i", argOn);
        client.writeDatagram(packetOn, QHostAddress::LocalHost, 58993);

        QTRY_COMPARE(project.isDuckingActive(), true);

        // Send ducking OFF: /glo/m/audio_d_s with int 0
        qint32 valOff = qToBigEndian<qint32>(0);
        QByteArray argOff(reinterpret_cast<const char*>(&valOff), sizeof(valOff));
        QByteArray packetOff = makeOscMessage("/glo/m/audio_d_s", "i", argOff);
        client.writeDatagram(packetOff, QHostAddress::LocalHost, 58993);

        QTRY_COMPARE(project.isDuckingActive(), false);

        osc.stopServer();
    }

    void testFuzzAndMalformedPackets()
    {
        OscServerController osc;
        osc.setInboundPort(58994);
        QVERIFY(osc.startServer());

        QUdpSocket client;
        // 1. Empty datagram
        client.writeDatagram(QByteArray(), QHostAddress::LocalHost, 58994);
        // 2. Random bytes
        client.writeDatagram(QByteArray("garbage!"), QHostAddress::LocalHost, 58994);
        // 3. Unterminated address
        client.writeDatagram(QByteArray("/abcdefg"), QHostAddress::LocalHost, 58994);
        // 4. Truncated float
        client.writeDatagram(QByteArray("/test\0\0\0,f\0\0", 12), QHostAddress::LocalHost, 58994);
        // 5. Oversized packet
        QByteArray oversized(8200, 'x');
        client.writeDatagram(oversized, QHostAddress::LocalHost, 58994);
        // 6. Ignored bundle
        client.writeDatagram(QByteArray("#bundle\0\0\0\0\0\0\0\0\0", 16), QHostAddress::LocalHost, 58994);

        QTest::qWait(100);

        // Server should still be running and operational
        QCOMPARE(osc.isRunning(), true);

        // Now send valid packet to verify server is alive and functioning
        QByteArray valid = makeOscMessage("/ping", "i", QByteArray("\0\0\0\x01", 4));
        client.writeDatagram(valid, QHostAddress::LocalHost, 58994);

        QTRY_COMPARE(osc.packetsReceived(), 1);
        QCOMPARE(osc.lastMessageAddress(), QString("/ping"));

        osc.stopServer();
    }

    void testAudioDeviceControllerAvailableOutputs()
    {
        SoundboardProjectController project;
        QStringList outputs = project.availableAudioOutputs();
        QVERIFY(!outputs.isEmpty());
        QVERIFY(outputs.contains(tr("Default System Output")));

        project.setOutputDeviceName(outputs.first());
        QCOMPARE(project.outputDeviceName(), outputs.first());
    }

    void testAutoStartAndFeedbackSettings()
    {
        OscServerController osc;
        QSignalSpy autoStartSpy(&osc, &OscServerController::autoStartChanged);
        QSignalSpy feedbackSpy(&osc, &OscServerController::feedbackEnabledChanged);

        osc.setAutoStart(true);
        QCOMPARE(osc.autoStart(), true);
        QCOMPARE(autoStartSpy.count(), 1);

        osc.setFeedbackEnabled(false);
        QCOMPARE(osc.feedbackEnabled(), false);
        QCOMPARE(feedbackSpy.count(), 1);

        // Reset
        osc.setAutoStart(false);
        osc.setFeedbackEnabled(true);
    }

    void testTwoWayFeedbackAndSync()
    {
        QUdpSocket feedbackListener;
        QVERIFY(feedbackListener.bind(QHostAddress::LocalHost, 58995));

        SoundboardProjectController project;
        project.newProject();

        OscServerController osc;
        osc.setProjectController(&project);
        osc.setInboundPort(58996);
        osc.setOutboundHost("127.0.0.1");
        osc.setOutboundPort(58995);
        osc.setFeedbackEnabled(true);
        QVERIFY(osc.startServer());

        // Test manual syncClients
        osc.syncClients();
        QTRY_VERIFY(feedbackListener.hasPendingDatagrams());

        QByteArray rxDatagram;
        rxDatagram.resize(feedbackListener.pendingDatagramSize());
        feedbackListener.readDatagram(rxDatagram.data(), rxDatagram.size());
        QVERIFY(rxDatagram.startsWith("/glo/s/") || rxDatagram.startsWith("/castersoundboard/"));

        // Clear socket
        while (feedbackListener.hasPendingDatagrams()) {
            QByteArray discard;
            discard.resize(feedbackListener.pendingDatagramSize());
            feedbackListener.readDatagram(discard.data(), discard.size());
        }

        // Test receiving /glo/sync command triggers sync
        QUdpSocket remoteClient;
        QByteArray syncPacket = makeOscMessage("/glo/sync", "");
        remoteClient.writeDatagram(syncPacket, QHostAddress::LocalHost, 58996);

        QTRY_VERIFY(feedbackListener.hasPendingDatagrams());

        osc.stopServer();
    }

    void testReceiveCommandScrubbing()
    {
        SoundboardProjectController project;
        project.newProject();
        SoundboardGridModel *grid = project.tabModel()->currentGridModel();
        QVERIFY(grid != nullptr);
        SoundTileController *tile = grid->appendTile();
        QVERIFY(tile != nullptr);
        tile->onDurationChanged(20000); // 20s duration
        QCOMPARE(tile->durationMs(), 20000);

        OscServerController osc;
        osc.setProjectController(&project);
        osc.setInboundPort(58997);
        QVERIFY(osc.startServer());

        QUdpSocket client;
        // 1. Scrub percentage: /cbp/1/m/t_p_p with float 0.5f (50% = 10000ms)
        float pctVal = 0.5f;
        quint32 rawFloat;
        std::memcpy(&rawFloat, &pctVal, sizeof(float));
        rawFloat = qToBigEndian(rawFloat);
        QByteArray fArg(reinterpret_cast<const char*>(&rawFloat), sizeof(float));
        QByteArray packet1 = makeOscMessage("/cbp/1/m/t_p_p", "f", fArg);
        client.writeDatagram(packet1, QHostAddress::LocalHost, 58997);

        QTRY_COMPARE(tile->positionMs(), 10000);

        // 2. Direct ms seek: /cbp/1/m/pos with int32 7500ms
        qint32 posVal = qToBigEndian<qint32>(7500);
        QByteArray iArg(reinterpret_cast<const char*>(&posVal), sizeof(posVal));
        QByteArray packet2 = makeOscMessage("/cbp/1/m/pos", "i", iArg);
        client.writeDatagram(packet2, QHostAddress::LocalHost, 58997);

        QTRY_COMPARE(tile->positionMs(), 7500);

        osc.stopServer();
    }
};

QTEST_MAIN(TestOscServer)
#include "test_osc_server.moc"
