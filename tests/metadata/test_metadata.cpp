#include "CasterBoard.h"
#include "CasterPlayer.h"
#include "CasterPlayerState.h"
#include "CasterLabelColorPicker.h"
#include "CSS.h"
#include <QApplication>
#include <QDataStream>
#include <QGridLayout>
#include <QTimer>
#include <QUdpSocket>
#include <QTest>

class MetadataTest : public QObject {
    Q_OBJECT
    QList<QByteArray> packets;
    QUdpSocket receiver, sender;

    void capture(CasterBoard &board) {
        connect(&board, &CasterBoard::_updateOSCClient, this, [this](OscMessageComposer *message) {
            // Exercise real serialized OSC over UDP, not mocked composer calls.
            sender.writeDatagram(*message->getBytes(), QHostAddress::LocalHost, receiver.localPort());
            delete message;
            if (!receiver.hasPendingDatagrams()) receiver.waitForReadyRead(1000);
            while (receiver.hasPendingDatagrams()) {
                QByteArray packet(receiver.pendingDatagramSize(), '\0');
                receiver.readDatagram(packet.data(), packet.size());
                packets.append(packet);
            }
        });
    }
    static QByteArray padded(QByteArray value) {
        value.append('\0');
        while (value.size() % 4) value.append('\0');
        return value;
    }
    static QByteArray color(const QString &id, const QString &value) {
        return padded(("/cbp/" + id + "/m/color").toUtf8()) + padded(",s") + padded(value.toUtf8());
    }
    static QByteArray grid(const QString &id, int row, int column) {
        QByteArray packet = padded(("/cbp/" + id + "/m/grid").toUtf8()) + padded(",ii");
        QDataStream stream(&packet, QIODevice::Append);
        stream << qint32(row) << qint32(column);
        return packet;
    }
    static QByteArray saved(const CasterBoard &board) {
        QByteArray bytes;
        QDataStream stream(&bytes, QIODevice::WriteOnly);
        stream << board;
        return bytes;
    }
    void edit(CasterPlayerWidget *player, int hue, int action) {
        QTimer::singleShot(0, [hue, action] {
            auto picker = qobject_cast<CasterLabelColorPicker *>(QApplication::activeModalWidget());
            Q_ASSERT(picker);
            picker->set_slider_H_Value(hue);
            if (action == 0) picker->button_Accept_Clicked();
            else if (action == 1) picker->button_Cancel_Clicked();
            else picker->reject(); // Escape/window dismissal
        });
        player->openColorPicker();
    }
private slots:
    void initTestCase() {
        QVERIFY(receiver.bind(QHostAddress(QHostAddress::LocalHost), quint16(0)));
    }
    void defaultSyncAndLayout() {
        CasterBoard board;
        board.isCurrentBoard = true;
        capture(board);
        packets.clear();
        board.syncWithOSCClient();
        // 1 board label + (6 existing state messages + 2 metadata) per slot.
        QCOMPARE(packets.size(), 257);
        for (int i = 0; i < 32; ++i) {
            const QString id = board.int_to_player_key->value(i);
            QCOMPARE(packets.count(color(id, "#DADADA")), 1);
            QCOMPARE(packets.count(grid(id, i / 8, i % 8)), 1);
        }
        QVERIFY(packets.contains(grid("1", 0, 0)));
        QVERIFY(packets.contains(grid("8", 0, 7)));
        QVERIFY(packets.contains(grid("Z", 3, 0)));
        QVERIFY(packets.contains(grid(",", 3, 7)));
        // Prove metadata comes from actual layout, not a second mapping.
        auto player = board.players->value(",");
        auto layout = qobject_cast<QGridLayout *>(player->parentWidget()->layout());
        QVERIFY(layout);
        layout->removeWidget(player);
        layout->addWidget(player, 4, 2);
        packets.clear(); board.syncWithOSCClient();
        QVERIFY(packets.contains(grid(",", 4, 2)));
    }
    void commitCancelAndProfileRoundTrip() {
        CasterBoard board;
        board.isCurrentBoard = true;
        capture(board);
        auto player = board.players->value(",");
        packets.clear();
        edit(player, 60, 0);
        QCOMPARE(packets.size(), 1);
        QCOMPARE(packets.first(), color(",", player->displayColor()));
        QVERIFY(player->displayColor() != "#DADADA");
        const QString appearance = player->styleSheet();
        const QString committedColor = player->displayColor();
        const QByteArray profile = saved(board);
        packets.clear();
        edit(player, 120, 1);
        edit(player, 180, 2);
        QCOMPARE(packets.size(), 0);
        QCOMPARE(player->styleSheet(), appearance);
        QCOMPARE(saved(board), profile);
        edit(player, 60, 0); // unchanged representative colour
        QCOMPARE(packets.size(), 0);

        CasterBoard loaded;
        QDataStream input(profile);
        input >> loaded;
        loaded.reloadBoardFromPlayerStates();
        QCOMPARE(saved(loaded), profile);
        QCOMPARE(loaded.players->value(",")->displayColor(), committedColor);
        loaded.isCurrentBoard = true;
        board.isCurrentBoard = false;
        capture(loaded);
        packets.clear(); loaded.syncWithOSCClient();
        QVERIFY(packets.contains(color(",", committedColor)));
        packets.clear(); edit(player, 240, 0);
        QVERIFY(packets.isEmpty()); // inactive-board edit is not forwarded
        CasterBoard defaults;
        QDataStream reset(saved(defaults));
        reset >> loaded;
        loaded.reloadBoardFromPlayerStates();
        packets.clear(); loaded.syncWithOSCClient();
        QVERIFY(packets.contains(color(",", "#DADADA")));
    }
    void colorMatchesAppliedGradient() {
        CasterPlayerWidget player;
        player.setGraphicsEffect(nullptr);
        for (int hue : {0, 60, 180, 360}) {
            player.setStyleSheet(Generate_CSS(HSL_C(hue, 0.2, -0.1)));
            const QString color = player.displayColor();
            const QBrush background = player.palette().brush(QPalette::Window);
            const QGradient *gradient = background.gradient();
            QVERIFY(gradient);
            const QColor top = gradient->stops().first().second;
            QCOMPARE(color, top.name().toUpper());

            QImage rendered(player.size(), QImage::Format_ARGB32);
            rendered.fill(Qt::transparent);
            player.render(&rendered);
            const QColor pixel = rendered.pixelColor(2, 0);
            const QColor reported(player.displayColor());

            // Pixel centres can interpolate slightly beyond the first stop.
            QVERIFY(qAbs(pixel.red() - reported.red()) <= 1);
            QVERIFY(qAbs(pixel.green() - reported.green()) <= 1);
            QVERIFY(qAbs(pixel.blue() - reported.blue()) <= 1);
        }
        // CSS in an existing profile is authoritative even if sliders differ.
        *player.playerState->PlayerBackgroundCSS = Generate_CSS(HSL_C(180, 0, 0));
        player.reloadFromPlayerState();
        QVERIFY(player.displayColor() != "#DADADA");
        player.setStyleSheet("invalid");
        QCOMPARE(player.displayColor(), QString("#DADADA"));
    }
};
QTEST_MAIN(MetadataTest)
#include "test_metadata.moc"
