#include <QTest>
#include <QSignalSpy>
#include "HotkeyManager.h"
#include "SoundTileController.h"
#include "SoundboardProjectController.h"
#include "SoundboardGridModel.h"

class TestHotkeyManager : public QObject
{
    Q_OBJECT

private slots:
    void testHotkeyDispatch()
    {
        HotkeyManager manager;
        SoundTileController tile;
        tile.setFilePath("/dummy/sound.wav");
        tile.setHotkey("Ctrl+Alt+1");

        manager.registerTile(&tile);

        QSignalSpy triggerSpy(&manager, &HotkeyManager::hotkeyTriggered);

        // Simulate Ctrl+Alt+1 press
        QKeyEvent matchEvent(QEvent::KeyPress, Qt::Key_1, Qt::ControlModifier | Qt::AltModifier);
        QVERIFY(manager.handleKeyEvent(&matchEvent));
        QCOMPARE(triggerSpy.count(), 1);

        // Simulate unrelated key press
        QKeyEvent unmatchEvent(QEvent::KeyPress, Qt::Key_2, Qt::ControlModifier | Qt::AltModifier);
        QVERIFY(!manager.handleKeyEvent(&unmatchEvent));
        QCOMPARE(triggerSpy.count(), 1);
    }

    void testMuteMode_Issue76()
    {
        HotkeyManager manager;
        SoundTileController tile;
        tile.setFilePath("/dummy/sound.wav");
        tile.setHotkey("P"); // single key

        manager.registerTile(&tile);

        QSignalSpy triggerSpy(&manager, &HotkeyManager::hotkeyTriggered);

        // Before mute: P triggers sound
        QKeyEvent keyP(QEvent::KeyPress, Qt::Key_P, Qt::NoModifier);
        QVERIFY(manager.handleKeyEvent(&keyP));
        QCOMPARE(triggerSpy.count(), 1);

        // Enable mute (typing mode)
        manager.setIsMuted(true);
        QCOMPARE(manager.isMuted(), true);

        // While muted: P must NOT trigger sound!
        QVERIFY(!manager.handleKeyEvent(&keyP));
        QCOMPARE(triggerSpy.count(), 1); // count unchanged!

        // Unmute: P triggers again
        manager.setIsMuted(false);
        QVERIFY(manager.handleKeyEvent(&keyP));
        QCOMPARE(triggerSpy.count(), 2);
    }

    void testMonitorProject()
    {
        SoundboardProjectController project;
        project.newProject();
        SoundboardGridModel *grid = project.tabModel()->currentGridModel();
        QVERIFY(grid != nullptr);

        SoundTileController *tile = grid->appendTile();
        tile->setHotkey("Ctrl+Alt+S");

        HotkeyManager manager;
        manager.monitorProject(&project);

        QSignalSpy triggerSpy(&manager, &HotkeyManager::hotkeyTriggered);

        QKeyEvent event(QEvent::KeyPress, Qt::Key_S, Qt::ControlModifier | Qt::AltModifier);
        QVERIFY(manager.handleKeyEvent(&event));
        QCOMPARE(triggerSpy.count(), 1);
    }
};

QTEST_MAIN(TestHotkeyManager)
#include "test_hotkey_manager.moc"
