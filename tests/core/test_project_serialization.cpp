#include <QTest>
#include <QTemporaryDir>
#include "SoundboardProjectController.h"
#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"
#include "SoundTileController.h"

class TestProjectSerialization : public QObject
{
    Q_OBJECT

private slots:
    void testWorkspaceDefaults()
    {
        SoundboardProjectController project;
        QCOMPARE(project.windowWidth(), 1024);
        QCOMPARE(project.windowHeight(), 720);
        QCOMPARE(project.isFullScreen(), false);
        QCOMPARE(project.isMaximized(), false);
        QCOMPARE(project.tileScale(), 1.0);
        QCOMPARE(project.masterVolume(), 1.0);
        QCOMPARE(project.isDuckingActive(), false);
        QCOMPARE(project.duckingVolumeRatio(), 0.25);
        QCOMPARE(project.hotkeyScope(), static_cast<int>(SoundboardProjectController::TabScoped));
        QCOMPARE(project.stopAllHotkey(), QString("Space"));
        QCOMPARE(project.duckingHotkey(), QString("Ctrl+D"));
        QCOMPARE(project.tabModel()->count(), 1);
    }

    void testSaveAndLoadJsonProject()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        QString projectPath = tempDir.filePath("test_show.caster");

        // Set up original project
        SoundboardProjectController p1;
        p1.setWindowWidth(1920);
        p1.setWindowHeight(1080);
        p1.setIsFullScreen(false);
        p1.setIsMaximized(true);
        p1.setTileScale(1.35);
        p1.setMasterVolume(0.9);
        p1.setDuckingVolumeRatio(0.20);
        p1.setOutputDeviceName("Virtual Sink Output");
        p1.setHotkeyScope(SoundboardProjectController::Global);
        p1.setStopAllHotkey("F12");
        p1.setDuckingHotkey("Ctrl+Alt+D");

        // Tab 1: Jingles
        p1.tabModel()->renameTab(0, "Jingles");
        p1.tabModel()->setTabHotkey(0, "Alt+J");
        SoundboardGridModel *g1 = p1.tabModel()->gridModelAt(0);
        SoundTileController *t1 = g1->appendTile();
        t1->setFilePath("/sounds/jingle1.mp3");
        t1->setDisplayName("Main Jingle");
        t1->setHotkey("Ctrl+Alt+1");
        t1->setVolume(0.85);
        t1->setIsLooped(false);
        t1->setColor("#2ECC71");
        t1->setHasCueRegion(true);
        t1->setCueBeginMs(500);
        t1->setCueEndMs(8000);

        // Tab 2: SFX
        SoundboardGridModel *g2 = p1.tabModel()->addTab("SFX");
        p1.tabModel()->setTabHotkey(1, "Alt+S");
        SoundTileController *t2 = g2->appendTile();
        t2->setFilePath("/sounds/applause.wav");
        t2->setDisplayName("Crowd Applause");
        t2->setHotkey("F1");
        t2->setVolume(1.0);
        t2->setIsLooped(true);
        t2->setColor("#E67E22");

        p1.tabModel()->setCurrentIndex(1); // active tab is SFX

        // Save project to .caster JSON
        QVERIFY(p1.saveProject(projectPath));
        QVERIFY(QFile::exists(projectPath));

        // Load into brand new controller
        SoundboardProjectController p2;
        QVERIFY(p2.loadProject(projectPath));

        // Verify window settings
        QCOMPARE(p2.windowWidth(), 1920);
        QCOMPARE(p2.windowHeight(), 1080);
        QCOMPARE(p2.isFullScreen(), false);
        QCOMPARE(p2.isMaximized(), true);
        QCOMPARE(p2.tileScale(), 1.35);
        QCOMPARE(p2.masterVolume(), 0.9);
        QCOMPARE(p2.duckingVolumeRatio(), 0.20);
        QCOMPARE(p2.outputDeviceName(), QString("Virtual Sink Output"));
        QCOMPARE(p2.hotkeyScope(), static_cast<int>(SoundboardProjectController::Global));
        QCOMPARE(p2.stopAllHotkey(), QString("F12"));
        QCOMPARE(p2.duckingHotkey(), QString("Ctrl+Alt+D"));

        // Verify tabs
        QCOMPARE(p2.tabModel()->count(), 2);
        QCOMPARE(p2.tabModel()->currentIndex(), 1);
        QCOMPARE(p2.tabModel()->tabNameAt(0), QString("Jingles"));
        QCOMPARE(p2.tabModel()->tabHotkeyAt(0), QString("Alt+J"));
        QCOMPARE(p2.tabModel()->tabNameAt(1), QString("SFX"));
        QCOMPARE(p2.tabModel()->tabHotkeyAt(1), QString("Alt+S"));

        // Verify Tab 1 tiles
        SoundboardGridModel *p2_g1 = p2.tabModel()->gridModelAt(0);
        QCOMPARE(p2_g1->count(), 1);
        SoundTileController *p2_t1 = p2_g1->tileAt(0);
        QCOMPARE(p2_t1->filePath(), QString("/sounds/jingle1.mp3"));
        QCOMPARE(p2_t1->displayName(), QString("Main Jingle"));
        QCOMPARE(p2_t1->hotkey(), QString("Ctrl+Alt+1"));
        QCOMPARE(p2_t1->volume(), 0.85);
        QCOMPARE(p2_t1->isLooped(), false);
        QCOMPARE(p2_t1->color(), QString("#2ECC71"));
        QCOMPARE(p2_t1->hasCueRegion(), true);
        QCOMPARE(p2_t1->cueBeginMs(), 500);
        QCOMPARE(p2_t1->cueEndMs(), 8000);

        // Verify Tab 2 tiles
        SoundboardGridModel *p2_g2 = p2.tabModel()->gridModelAt(1);
        QCOMPARE(p2_g2->count(), 1);
        SoundTileController *p2_t2 = p2_g2->tileAt(0);
        QCOMPARE(p2_t2->filePath(), QString("/sounds/applause.wav"));
        QCOMPARE(p2_t2->displayName(), QString("Crowd Applause"));
        QCOMPARE(p2_t2->hotkey(), QString("F1"));
        QCOMPARE(p2_t2->volume(), 1.0);
        QCOMPARE(p2_t2->isLooped(), true);
        QCOMPARE(p2_t2->color(), QString("#E67E22"));
    }

    void testLegacyProjectFallback()
    {
        // JSON without "shortcuts" object should default to Space and Ctrl+D
        QJsonObject root;
        root["formatVersion"] = 2;
        SoundboardProjectController project;
        QVERIFY(project.fromJson(root));
        QCOMPARE(project.stopAllHotkey(), QString("Space"));
        QCOMPARE(project.duckingHotkey(), QString("Ctrl+D"));
    }

    void testDuckingToggle()
    {
        SoundboardProjectController project;
        QCOMPARE(project.isDuckingActive(), false);

        project.toggleAudioDucking();
        QCOMPARE(project.isDuckingActive(), true);

        project.toggleAudioDucking();
        QCOMPARE(project.isDuckingActive(), false);
    }
};

QTEST_MAIN(TestProjectSerialization)
#include "test_project_serialization.moc"
