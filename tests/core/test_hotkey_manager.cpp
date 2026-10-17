#include <QTest>
#include <QSignalSpy>
#include "HotkeyManager.h"
#include "SoundTileController.h"
#include "SoundboardProjectController.h"
#include "SoundboardGridModel.h"
#include "ActionsController.h"

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

    void testKeySequenceFormatting()
    {
        HotkeyManager manager;
        QCOMPARE(manager.keySequenceToString(Qt::Key_P, Qt::NoModifier), QString("P"));
        QCOMPARE(manager.keySequenceToString(Qt::Key_1, Qt::ControlModifier | Qt::AltModifier), QString("Ctrl+Alt+1"));
        QCOMPARE(manager.keySequenceToString(Qt::Key_Space, Qt::NoModifier), QString("Space"));
        QCOMPARE(manager.keySequenceToString(Qt::Key_F5, Qt::NoModifier), QString("F5"));
        QCOMPARE(manager.keySequenceToString(Qt::Key_Control, Qt::ControlModifier), QString()); // modifier alone returns empty
        QCOMPARE(manager.modifiersToString(Qt::ControlModifier | Qt::AltModifier), QString("Ctrl+Alt+..."));
    }

    void testTabHotkeySwitching()
    {
        SoundboardProjectController project;
        project.newProject();
        project.tabModel()->renameTab(0, "Main");
        project.tabModel()->addTab("SFX");
        project.tabModel()->setTabHotkey(1, "F2");

        HotkeyManager manager;
        manager.monitorProject(&project);

        QSignalSpy tabSpy(&manager, &HotkeyManager::tabHotkeyTriggered);
        QCOMPARE(project.tabModel()->currentIndex(), 0);

        // Switch to Tab 1 via custom F2 hotkey
        QKeyEvent eventF2(QEvent::KeyPress, Qt::Key_F2, Qt::NoModifier);
        QVERIFY(manager.handleKeyEvent(&eventF2));
        QCOMPARE(project.tabModel()->currentIndex(), 1);
        QCOMPARE(tabSpy.count(), 1);

        // Switch back to Tab 0 via default Alt+1 hotkey
        QKeyEvent eventAlt1(QEvent::KeyPress, Qt::Key_1, Qt::AltModifier);
        QVERIFY(manager.handleKeyEvent(&eventAlt1));
        QCOMPARE(project.tabModel()->currentIndex(), 0);
        QCOMPARE(tabSpy.count(), 2);
    }

    void testBankModeTabScopedVsGlobal()
    {
        SoundboardProjectController project;
        project.newProject();
        project.tabModel()->renameTab(0, "Bank 1");
        SoundboardGridModel *g1 = project.tabModel()->gridModelAt(0);
        SoundTileController *t1 = g1->appendTile();
        t1->setHotkey("P");

        SoundboardGridModel *g2 = project.tabModel()->addTab("Bank 2");
        SoundTileController *t2 = g2->appendTile();
        t2->setHotkey("Q");

        HotkeyManager manager;
        manager.monitorProject(&project);

        QSignalSpy tileSpy(&manager, &HotkeyManager::hotkeyTriggered);

        // --- Mode B: TabScoped (Default) ---
        QCOMPARE(project.hotkeyScope(), static_cast<int>(SoundboardProjectController::TabScoped));
        project.tabModel()->setCurrentIndex(0); // Bank 1 active

        // Key P should fire t1 (on active Tab 0)
        QKeyEvent keyP(QEvent::KeyPress, Qt::Key_P, Qt::NoModifier);
        QVERIFY(manager.handleKeyEvent(&keyP));
        QCOMPARE(tileSpy.count(), 1);

        // Key Q should NOT fire t2 because t2 is on inactive Tab 1
        QKeyEvent keyQ(QEvent::KeyPress, Qt::Key_Q, Qt::NoModifier);
        QVERIFY(!manager.handleKeyEvent(&keyQ));
        QCOMPARE(tileSpy.count(), 1);

        // Switch to Bank 2 (Tab 1 active)
        project.tabModel()->setCurrentIndex(1);

        // Now Key P should NOT fire
        QVERIFY(!manager.handleKeyEvent(&keyP));
        QCOMPARE(tileSpy.count(), 1);

        // But Key Q SHOULD fire
        QVERIFY(manager.handleKeyEvent(&keyQ));
        QCOMPARE(tileSpy.count(), 2);

        // --- Mode A: Global Mode ---
        project.setHotkeyScope(SoundboardProjectController::Global);
        QCOMPARE(project.hotkeyScope(), static_cast<int>(SoundboardProjectController::Global));

        // In global mode, while Tab 1 is active, BOTH P (Tab 0) and Q (Tab 1) fire!
        QVERIFY(manager.handleKeyEvent(&keyP));
        QCOMPARE(tileSpy.count(), 3);

        QVERIFY(manager.handleKeyEvent(&keyQ));
        QCOMPARE(tileSpy.count(), 4);
    }

    void testGlobalActionShortcuts()
    {
        SoundboardProjectController project;
        project.newProject();

        HotkeyManager manager;
        manager.monitorProject(&project);

        QSignalSpy stopAllSpy(&manager, &HotkeyManager::stopAllTriggered);
        QSignalSpy duckingSpy(&manager, &HotkeyManager::duckingTriggered);

        // 1. Defaults: Space = Stop All, Ctrl+D = Toggle Ducking
        QCOMPARE(project.stopAllHotkey(), QString("Space"));
        QCOMPARE(project.duckingHotkey(), QString("Ctrl+D"));

        QCOMPARE(project.isDuckingActive(), false);

        // Press Space -> triggers stop all
        QKeyEvent spaceKey(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
        QVERIFY(manager.handleKeyEvent(&spaceKey));
        QCOMPARE(stopAllSpy.count(), 1);

        // Press Ctrl+D -> triggers ducking toggle
        QKeyEvent duckKey(QEvent::KeyPress, Qt::Key_D, Qt::ControlModifier);
        QVERIFY(manager.handleKeyEvent(&duckKey));
        QCOMPARE(duckingSpy.count(), 1);
        QCOMPARE(project.isDuckingActive(), true);

        // Press Ctrl+D again -> un-ducks
        QVERIFY(manager.handleKeyEvent(&duckKey));
        QCOMPARE(duckingSpy.count(), 2);
        QCOMPARE(project.isDuckingActive(), false);

        // 2. Custom hotkeys: Stop All = F12, Ducking = Ctrl+Alt+K
        project.setStopAllHotkey("F12");
        project.setDuckingHotkey("Ctrl+Alt+K");

        // Old keys should not trigger anymore
        QVERIFY(!manager.handleKeyEvent(&spaceKey));
        QCOMPARE(stopAllSpy.count(), 1);
        QVERIFY(!manager.handleKeyEvent(&duckKey));
        QCOMPARE(duckingSpy.count(), 2);

        // New keys should trigger
        QKeyEvent f12Key(QEvent::KeyPress, Qt::Key_F12, Qt::NoModifier);
        QVERIFY(manager.handleKeyEvent(&f12Key));
        QCOMPARE(stopAllSpy.count(), 2);

        QKeyEvent customDuckKey(QEvent::KeyPress, Qt::Key_K, Qt::ControlModifier | Qt::AltModifier);
        QVERIFY(manager.handleKeyEvent(&customDuckKey));
        QCOMPARE(duckingSpy.count(), 3);
        QCOMPARE(project.isDuckingActive(), true);

        // 3. Muted mode (Issue #76) should mute global action shortcuts too
        manager.setIsMuted(true);
        QVERIFY(!manager.handleKeyEvent(&f12Key));
        QCOMPARE(stopAllSpy.count(), 2);
        QVERIFY(!manager.handleKeyEvent(&customDuckKey));
        QCOMPARE(duckingSpy.count(), 3);

        manager.setIsMuted(false);
        QVERIFY(manager.handleKeyEvent(&f12Key));
        QCOMPARE(stopAllSpy.count(), 3);

        // 4. Cleared hotkeys should not trigger
        project.setStopAllHotkey("");
        QVERIFY(!manager.handleKeyEvent(&f12Key));
        QCOMPARE(stopAllSpy.count(), 3);
    }

    void testSelfTransmittingKeySuppression()
    {
        SoundboardProjectController project;
        HotkeyManager manager;
        manager.monitorProject(&project);

        SoundTileController tile;
        tile.setFilePath("/dummy/sound.wav");
        tile.setHotkey("V");

        manager.registerTile(&tile);

        QSignalSpy triggerSpy(&manager, &HotkeyManager::hotkeyTriggered);

        // Physical V press triggers tile
        QKeyEvent keyV(QEvent::KeyPress, Qt::Key_V, Qt::NoModifier);
        QVERIFY(manager.handleKeyEvent(&keyV));
        QCOMPARE(triggerSpy.count(), 1);

        // Active transmission suppresses hotkey re-trigger
        ActionsController *actions = project.actionsController();
        QVERIFY(actions != nullptr);
        actions->testTriggerKey("KEY_V", 1000);
        QVERIFY(actions->isTransmittingKey("KEY_V"));

        QVERIFY(!manager.handleKeyEvent(&keyV));
        QCOMPARE(triggerSpy.count(), 1);

        actions->stopAll();
        QVERIFY(!actions->isTransmitting());

        QVERIFY(manager.handleKeyEvent(&keyV));
        QCOMPARE(triggerSpy.count(), 2);
    }
};

QTEST_MAIN(TestHotkeyManager)
#include "test_hotkey_manager.moc"
