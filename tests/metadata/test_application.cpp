#include "MainWindow.h"
#include "CasterBoard.h"
#include "CasterPlayer.h"
#include "CasterLabelColorPicker.h"
#include "CasterOSCServerConfigPicker.h"
#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QTabWidget>
#include <QTimer>
#include <QUdpSocket>
#include <QTest>
#include <QProcess>
#include <QFileInfo>
#include <QDir>

class ApplicationTest : public QObject {
    Q_OBJECT
private slots:
    void syncTabsDisabledAndHardening() {
        QUdpSocket receiver;
        QVERIFY(receiver.bind(QHostAddress(QHostAddress::LocalHost), quint16(0)));
        receiver.setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 1024);
        QList<QByteArray> packets;
        connect(&receiver, &QUdpSocket::readyRead, [&] {
            while (receiver.hasPendingDatagrams()) {
                QByteArray data(receiver.pendingDatagramSize(), '\0');
                receiver.readDatagram(data.data(), data.size());
                packets.append(data);
            }
        });
        MainWindow window;
        auto addTab = [&] {
            QTimer::singleShot(0, [] {
                auto dialog = qobject_cast<QInputDialog *>(QApplication::activeModalWidget());
                Q_ASSERT(dialog);
                dialog->setTextValue("Metadata test"); dialog->accept();
            });
            window.addNewTab();
        };
        addTab(); addTab();
        auto tabs = window.findChild<QTabWidget *>();
        QVERIFY(tabs); QCOMPARE(tabs->count(), 2);
        auto first = qobject_cast<CasterBoard *>(tabs->widget(0));
        auto second = qobject_cast<CasterBoard *>(tabs->widget(1));
        tabs->setCurrentIndex(1);
        auto player = second->players->value(",");
        QTimer::singleShot(0, [] {
            auto dialog = qobject_cast<CasterLabelColorPicker *>(QApplication::activeModalWidget());
            Q_ASSERT(dialog);
            dialog->set_slider_H_Value(60); dialog->button_Accept_Clicked();
        });
        player->openColorPicker();
        first->syncWithOSCClient(); second->syncWithOSCClient();
        QTest::qWait(30); QVERIFY(packets.isEmpty()); // disabled

        // Allocate an unused listener port, and configure through the real dialog.
        QUdpSocket probe;
        QVERIFY(probe.bind(QHostAddress(QHostAddress::LocalHost), quint16(0)));
        const quint16 inboundPort = probe.localPort(); probe.close();
        QString localhost("127.0.0.1");
        QTimer dialogs;
        connect(&dialogs, &QTimer::timeout, [&] {
            if (auto config = qobject_cast<CasterOSCServerConfigPicker *>(QApplication::activeModalWidget())) {
                config->setInBoundPort(inboundPort);
                config->setOutBoundIPv4(&localhost);
                config->setOutBoundPort(receiver.localPort());
                config->ok = true;
                config->Result = CasterOSCServerConfigPicker::Action_Start_Server;
                config->accept();
            } else if (auto message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                message->accept();
            }
        });
        dialogs.start(10); window.openOSCSettings(); dialogs.stop();
        QTRY_VERIFY(packets.size() >= 258);
        QTest::qWait(30); packets.clear();
        auto requestSync = [&] {
            OscMessageComposer sync("/glo/sync"); sync.pushFloat(1.0f);
            receiver.writeDatagram(*sync.getBytes(), QHostAddress::LocalHost, inboundPort);
        };
        requestSync(); QTRY_COMPARE(packets.size(), 258);
        auto contains = [&](const QByteArray &address, const QByteArray &value) {
            for (const auto &packet : packets)
                if (packet.startsWith(address + '\0') && packet.contains(value)) return true;
            return false;
        };
        QVERIFY(contains("/cbp/,/m/color", player->displayColor().toUtf8()));
        packets.clear(); tabs->setCurrentIndex(0);
        QTRY_COMPARE(packets.size(), 257);
        QVERIFY(contains("/cbp/,/m/color", "#DADADA"));
        packets.clear(); tabs->setCurrentIndex(1);
        QTRY_COMPARE(packets.size(), 257);
        QVERIFY(contains("/cbp/,/m/color", player->displayColor().toUtf8()));

        // Incoming metadata must remain read-only.
        const QString before = player->styleSheet();
        OscMessageComposer writeColor("/cbp/,/m/color"); writeColor.pushString("#000000");
        receiver.writeDatagram(*writeColor.getBytes(), QHostAddress::LocalHost, inboundPort);
        OscMessageComposer writeGrid("/cbp/,/m/grid"); writeGrid.pushInt32(99); writeGrid.pushInt32(99);
        receiver.writeDatagram(*writeGrid.getBytes(), QHostAddress::LocalHost, inboundPort);
        QTest::qWait(30); QCOMPARE(player->styleSheet(), before);

        // Run the local hardening regression sender when that separate patch is present.
        const QString script = QFileInfo(QString::fromUtf8(__FILE__)).dir().filePath("../osc/test_osc_packets.py");
        if (QFileInfo::exists(script)) {
            QProcess process;
            process.start("python3", {script, "127.0.0.1", QString::number(inboundPort)});
            QTRY_VERIFY_WITH_TIMEOUT(process.state() == QProcess::NotRunning, 10000);
            QCOMPARE(process.exitCode(), 0);
            QCOMPARE(process.exitStatus(), QProcess::NormalExit);
        }
        packets.clear(); requestSync(); QTRY_COMPARE(packets.size(), 258);
        QVERIFY(contains("/cbp/,/m/color", player->displayColor().toUtf8()));
        // Existing older-client command still works.
        OscMessageComposer volume("/cbp/,/m/vol"); volume.pushFloat(0.42f);
        receiver.writeDatagram(*volume.getBytes(), QHostAddress::LocalHost, inboundPort);
        QTRY_COMPARE(player->player->volume(), 42);
    }
};
QTEST_MAIN(ApplicationTest)
#include "test_application.moc"
