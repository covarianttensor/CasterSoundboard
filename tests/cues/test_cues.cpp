#include "CasterPlayer.h"
#include "CasterPlayerState.h"
#include "CasterCuePicker.h"
#include "libs/osc/composer/OscMessageComposer.h"
#include <QTest>
#include <QTemporaryFile>
#include <QDataStream>
#include <QFileInfo>
#include <QDir>
#include <QLabel>
#include <QPushButton>

class CueTest : public QObject {
    Q_OBJECT
private slots:
    void stoppedInvalidCueDoesNotSpin_data() {
        QTest::addColumn<int>("start");
        QTest::addColumn<int>("stop");
        QTest::newRow("equal") << 500 << 500;
        QTest::newRow("reversed") << 800 << 300;
        QTest::newRow("past-end") << 3000 << 4000;
        QTest::newRow("negative") << -100 << 500;
    }
    void stoppedInvalidCueDoesNotSpin() {
        QFETCH(int, start);
        QFETCH(int, stop);
        QTemporaryFile wav("/tmp/caster-cue-XXXXXX.wav");
        QVERIFY(wav.open());
        QDataStream out(&wav); out.setByteOrder(QDataStream::LittleEndian);
        wav.write("RIFF",4); out << quint32(32036); wav.write("WAVEfmt ",8);
        out << quint32(16) << quint16(1) << quint16(1) << quint32(8000)
            << quint32(16000) << quint16(2) << quint16(16);
        wav.write("data",4); out << quint32(32000); wav.write(QByteArray(32000,0)); wav.flush();
        CasterPlayerWidget widget;
        connect(&widget, &CasterPlayerWidget::updateOSCClient, [](OscMessageComposer *message) { delete message; });
        widget.player->setMedia(QUrl::fromLocalFile(wav.fileName()));
        QTRY_VERIFY(widget.player->duration() > 0);
        widget.playerState->timeSet = true;
        widget.playerState->startTime = start;
        widget.playerState->stopTime = stop;
        int notifications = 0;
        connect(widget.player, &QMediaPlayer::positionChanged, [&] { ++notifications; });
        widget.player->setPosition(500);
        QTest::qWait(200);
        QVERIFY2(notifications < 30, qPrintable(QString("Idle position notifications: %1").arg(notifications)));
        QCOMPARE(widget.player->state(), QMediaPlayer::StoppedState);
        QCOMPARE(widget.playerState->startTime, 0);
        QCOMPARE(qint64(widget.playerState->stopTime), widget.player->duration());

        // Valid cues survive duration changes and stop at the cue end.
        widget.playerState->timeSet = true;
        widget.playerState->startTime = 200;
        widget.playerState->stopTime = 700;
        widget.playerDurationChanged(0);
        QCOMPARE(widget.playerState->startTime, 200);
        widget.playerDurationChanged(widget.player->duration());
        QCOMPARE(widget.playerState->stopTime, 700);
        widget.player->setVolume(0);
        widget.player->setNotifyInterval(20);
        widget.playSound();
        QTRY_COMPARE(widget.player->state(), QMediaPlayer::PlayingState);
        QTRY_COMPARE(widget.player->state(), QMediaPlayer::StoppedState);
        notifications = 0;
        QTest::qWait(200);
        QVERIFY(notifications < 5);

        // A valid loop continues across several cue boundaries.
        widget.playerState->loop = true;
        widget.playSound();
        int wraps = 0;
        qint64 previous = 0;
        auto connection = connect(widget.player, &QMediaPlayer::positionChanged, [&](qint64 position) {
            if (position < previous) ++wraps;
            previous = position;
        });
        QTRY_VERIFY(wraps >= 2);
        QCOMPARE(widget.player->state(), QMediaPlayer::PlayingState);
        disconnect(connection);
        widget.player->stop();
        QTest::qWait(100);
        QCOMPARE(widget.player->state(), QMediaPlayer::StoppedState);

        // Full-track looping also works when the backend reaches EndOfMedia.
        widget.playerState->startTime = 0;
        widget.playerState->stopTime = widget.player->duration();
        wraps = 0; previous = 0;
        connection = connect(widget.player, &QMediaPlayer::positionChanged, [&](qint64 position) {
            if (position < previous) ++wraps;
            previous = position;
        });
        widget.playSound();
        QTRY_VERIFY(wraps >= 1);
        QTRY_COMPARE(widget.player->state(), QMediaPlayer::PlayingState);
        disconnect(connection);
        widget.playerState->loop = false;
        widget.player->stop();
    }

    void uiLoopButtonAndChimeFileLoops() {
        const QString chimePath = QFileInfo(QString::fromUtf8(__FILE__)).dir().filePath("test_loop_chime.wav");
        QVERIFY2(QFileInfo::exists(chimePath), qPrintable(chimePath));

        CasterPlayerWidget widget;
        widget.setHotKeyLetter("1");
        widget.show();
        connect(&widget, &CasterPlayerWidget::updateOSCClient, [](OscMessageComposer *message) { delete message; });

        // Simulate loading the sound effect and clicking the UI loop button.
        *widget.playerState->filePath = chimePath;
        widget.reloadFromPlayerState();
        QTRY_VERIFY(widget.player->duration() > 0);
        QCOMPARE(widget.soundNameLabel->text(), QString("test_loop_chime"));
        QCOMPARE(widget.playerState->loop, false);

        QTest::mouseClick(widget.toggleLoopButton, Qt::LeftButton);
        QCOMPARE(widget.playerState->loop, true);

        widget.player->setVolume(0);
        int wraps = 0;
        qint64 previous = 0;
        auto connection = connect(widget.player, &QMediaPlayer::positionChanged, [&](qint64 position) {
            if (position < previous) ++wraps;
            previous = position;
        });

        // Start playback by clicking the widget tile itself.
        QTest::mousePress(&widget, Qt::LeftButton);
        QTRY_COMPARE(widget.player->state(), QMediaPlayer::PlayingState);
        QTRY_VERIFY_WITH_TIMEOUT(wraps >= 2, 10000);
        QCOMPARE(widget.player->state(), QMediaPlayer::PlayingState);

        // Capture a screenshot of the active looping tile for visual verification.
        const QString screenshotPath = qEnvironmentVariable("CASTER_LOOP_SCREENSHOT");
        if (!screenshotPath.isEmpty()) {
            QVERIFY(widget.grab().save(screenshotPath));
        }

        disconnect(connection);
        widget.stopSound();
        QTRY_COMPARE(widget.player->state(), QMediaPlayer::StoppedState);
    }
};
QTEST_MAIN(CueTest)
#include "test_cues.moc"
