#include <QTest>
#include <QSignalSpy>
#include <QJsonObject>
#include "SoundTileController.h"

class TestSoundTile : public QObject
{
    Q_OBJECT

private slots:
    void testInitialDefaults()
    {
        SoundTileController tile;
        QVERIFY(!tile.id().isEmpty());
        QCOMPARE(tile.filePath(), QString());
        QCOMPARE(tile.displayName(), QString());
        QCOMPARE(tile.hotkey(), QString());
        QCOMPARE(tile.volume(), 1.0);
        QCOMPARE(tile.isLooped(), false);
        QCOMPARE(tile.triggerStyle(), SoundTileController::PlayPause);
        QCOMPARE(tile.hasCueRegion(), false);
        QCOMPARE(tile.cueBeginMs(), 0);
        QCOMPARE(tile.cueEndMs(), 0);
        QCOMPARE(tile.color(), QString("#3A3D40"));
        QCOMPARE(tile.playbackState(), QMediaPlayer::StoppedState);
    }

    void testAutoDisplayName()
    {
        SoundTileController tile;
        tile.setFilePath("/home/user/Music/Applause_Track.mp3");
        QCOMPARE(tile.displayName(), QString("Applause_Track"));

        // Manual rename should not be overwritten
        tile.setDisplayName("Custom Stinger");
        QCOMPARE(tile.displayName(), QString("Custom Stinger"));
    }

    void testHotkeySequences()
    {
        SoundTileController tile;
        tile.setHotkey("Ctrl+Alt+1");
        QCOMPARE(tile.keySequence(), QKeySequence("Ctrl+Alt+1"));
        QCOMPARE(tile.hotkey(), QString("Ctrl+Alt+1"));

        // Single key out of the way (Issue #76)
        tile.setHotkey("P");
        QCOMPARE(tile.hotkey(), QString("P"));

        tile.setHotkey("Shift+F5");
        QCOMPARE(tile.keySequence(), QKeySequence("Shift+F5"));
    }

    void testVolumeAndDucking()
    {
        SoundTileController tile;
        tile.setVolume(0.8);
        QCOMPARE(tile.volume(), 0.8);

        // Clamping
        tile.setVolume(1.5);
        QCOMPARE(tile.volume(), 1.0);
        tile.setVolume(-0.5);
        QCOMPARE(tile.volume(), 0.0);
    }

    void testCuePoints()
    {
        SoundTileController tile;
        tile.setHasCueRegion(true);
        tile.setCueBeginMs(500);
        tile.setCueEndMs(4500);

        QCOMPARE(tile.hasCueRegion(), true);
        QCOMPARE(tile.cueBeginMs(), 500);
        QCOMPARE(tile.cueEndMs(), 4500);
    }

    void testJsonSerializationRoundtrip()
    {
        SoundTileController tile1("test-tile-id-123");
        tile1.setFilePath("/path/to/horn.wav");
        tile1.setDisplayName("Air Horn");
        tile1.setHotkey("Ctrl+Alt+H");
        tile1.setVolume(0.75);
        tile1.setIsLooped(true);
        tile1.setTriggerStyle(SoundTileController::PlayStop);
        tile1.setColor("#E74C3C");
        tile1.setHasCueRegion(true);
        tile1.setCueBeginMs(250);
        tile1.setCueEndMs(3500);

        QJsonObject json = tile1.toJson();

        SoundTileController tile2;
        QVERIFY(tile2.fromJson(json));

        QCOMPARE(tile2.id(), QString("test-tile-id-123"));
        QCOMPARE(tile2.filePath(), QString("/path/to/horn.wav"));
        QCOMPARE(tile2.displayName(), QString("Air Horn"));
        QCOMPARE(tile2.hotkey(), QString("Ctrl+Alt+H"));
        QCOMPARE(tile2.volume(), 0.75);
        QCOMPARE(tile2.isLooped(), true);
        QCOMPARE(tile2.triggerStyle(), SoundTileController::PlayStop);
        QCOMPARE(tile2.color(), QString("#E74C3C"));
        QCOMPARE(tile2.hasCueRegion(), true);
        QCOMPARE(tile2.cueBeginMs(), 250);
        QCOMPARE(tile2.cueEndMs(), 3500);
    }

    void testVideoSupport()
    {
        SoundTileController tile;
        QCOMPARE(tile.hasVideo(), false);
        QCOMPARE(tile.videoOutput(), nullptr);

        QObject dummyOutput;
        tile.setVideoOutput(&dummyOutput);
        QCOMPARE(tile.videoOutput(), &dummyOutput);
    }
};

QTEST_MAIN(TestSoundTile)
#include "test_sound_tile.moc"
