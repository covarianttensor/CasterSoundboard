#include <QTest>
#include <QTemporaryDir>
#include <QDataStream>
#include "SoundboardProjectController.h"
#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"
#include "SoundTileController.h"
#include "PttController.h"

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
        QCOMPARE(project.openProjectHotkey(), QString("Ctrl+O"));
        QCOMPARE(project.saveProjectHotkey(), QString("Ctrl+S"));
        QCOMPARE(project.saveAsProjectHotkey(), QString("Ctrl+Shift+S"));
        QCOMPARE(project.newTabHotkey(), QString("Ctrl+N"));
        QCOMPARE(project.importDirectoryHotkey(), QString("Ctrl+I"));
        QCOMPARE(project.renameTabHotkey(), QString("F2"));
        QCOMPARE(project.pttDialogHotkey(), QString("Ctrl+Shift+P"));
        QCOMPARE(project.muteHotkeysHotkey(), QString("Ctrl+M"));
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
        p1.setOpenProjectHotkey("Ctrl+Alt+O");
        p1.setSaveProjectHotkey("Ctrl+Alt+S");
        p1.setSaveAsProjectHotkey("Ctrl+Alt+Shift+S");
        p1.setNewTabHotkey("Ctrl+T");
        p1.setImportDirectoryHotkey("Ctrl+Shift+I");
        p1.setRenameTabHotkey("F6");
        p1.setPttDialogHotkey("F8");
        p1.setMuteHotkeysHotkey("Ctrl+Shift+M");

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
        QCOMPARE(p2.openProjectHotkey(), QString("Ctrl+Alt+O"));
        QCOMPARE(p2.saveProjectHotkey(), QString("Ctrl+Alt+S"));
        QCOMPARE(p2.saveAsProjectHotkey(), QString("Ctrl+Alt+Shift+S"));
        QCOMPARE(p2.newTabHotkey(), QString("Ctrl+T"));
        QCOMPARE(p2.importDirectoryHotkey(), QString("Ctrl+Shift+I"));
        QCOMPARE(p2.renameTabHotkey(), QString("F6"));
        QCOMPARE(p2.pttDialogHotkey(), QString("F8"));
        QCOMPARE(p2.muteHotkeysHotkey(), QString("Ctrl+Shift+M"));

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

    void testBinaryLegacyImport()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath("legacy.caster");
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_0);
        out << QString("Old board");
        // Alphabetical QMap order starts with comma, then 1 through 8.
        for (int i = 0; i < 32; ++i)
            out << QString() << qint32(i == 1 ? 37 : 100)
                << qint32(100) << qint32(850) << true << true
                << qint32(-88) << qint32(0) << qint32(0)
                << QString("rgba(188.733,252,184,255)") << true;
        out << quint32(0x43414354) << quint32(2) << quint32(0) << qint32(-1);
        out << quint32(0x4357535a) << quint32(1) << QSize(1200, 800);
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(data), data.size());
        file.close();

        SoundboardProjectController project;
        // Verify initial window size
        QCOMPARE(project.windowWidth(), 1024);
        QCOMPARE(project.windowHeight(), 720);

        QVERIFY(project.loadProject(path));
        QCOMPARE(project.tabModel()->tabNameAt(0), QString("Old board"));
        auto *grid = project.tabModel()->gridModelAt(0);
        QCOMPARE(grid->count(), 32);
        auto *tile = grid->tileAt(0);
        QCOMPARE(tile->hotkey(), QString("1"));
        QCOMPARE(tile->volume(), 0.37);
        QVERIFY(tile->isLooped());
        QVERIFY(tile->hasCueRegion());
        QCOMPARE(tile->cueBeginMs(), 100);
        QCOMPARE(tile->cueEndMs(), 850);
        QCOMPARE(tile->color(), QString("#bdfcb8"));
        QCOMPARE(grid->tileAt(31)->hotkey(), QString(","));

        // Crucial test: Window size from legacy CWSZ MUST NOT be imported or alter window dimensions
        QCOMPARE(project.windowWidth(), 1024);
        QCOMPARE(project.windowHeight(), 720);

        QVERIFY(project.filePath().isEmpty());
        QVERIFY(project.isDirty());
        QVERIFY(!project.save());
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), data);
        file.close();
        QVERIFY(project.saveProject(dir.filePath("converted.caster")));
        SoundboardProjectController reloaded;
        QVERIFY(reloaded.loadProject(dir.filePath("converted.caster")));
        QCOMPARE(reloaded.tabModel()->gridModelAt(0)->tileAt(0)->volume(), 0.37);

        // Failed imports must not replace the current board.
        for (const auto &bad : {data.left(100), data.left(data.size() - 1), data + "junk"}) {
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
            file.write(bad);
            file.close();
            QVERIFY(!project.loadProject(path));
            QCOMPARE(project.tabModel()->tabNameAt(0), QString("Old board"));
        }
        // Original v1 files had no extension blocks.
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        file.write(data.left(data.size() - 32));
        file.close();
        QVERIFY(project.loadProject(path));
    }

    void testLegacyImportWithActions()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath("legacy_with_actions.caster");
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_0);
        out << QString("Gaming Board");
        for (int i = 0; i < 32; ++i)
            out << QString("/sound.mp3") << qint32(100)
                << qint32(0) << qint32(0) << false << false
                << qint32(0) << qint32(0) << qint32(0)
                << QString() << false;
        // CACT block version 2 with 1 action
        out << quint32(0x43414354) << quint32(2) << quint32(1) << qint32(0);
        out << QString("Overwatch PTT") << qint32(1) << QString("KEY_V") << qint32(200);
        out << QString("KEY_M") << true; // attached = true
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(data);
        file.close();

        SoundboardProjectController project;
        QVERIFY(project.loadProject(path));
        QCOMPARE(project.tabModel()->tabNameAt(0), QString("Gaming Board"));
        QVERIFY(project.pttController() != nullptr);
        QCOMPARE(project.pttController()->count(), 1);
        QVERIFY(project.pttController()->hasPttForTab("Gaming Board"));
        QCOMPARE(project.pttController()->activeBadgeForTab("Gaming Board"), QString("PTT: V"));
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

    void testDirtyStateTracking()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        QString projectPath = tempDir.filePath("dirty_test.caster");

        SoundboardProjectController project;
        // Starts clean on new project
        QCOMPARE(project.isDirty(), false);
        QCOMPARE(project.filePath(), QString());

        // Adding a tile marks dirty
        auto *tile = project.tabModel()->gridModelAt(0)->appendTile();
        QCOMPARE(project.isDirty(), true);

        // Saving marks clean
        QVERIFY(project.saveProject(projectPath));
        QCOMPARE(project.isDirty(), false);
        QCOMPARE(project.filePath(), projectPath);

        // Modifying tile volume marks dirty
        tile->setVolume(0.5);
        QCOMPARE(project.isDirty(), true);

        // Saving again marks clean
        QVERIFY(project.saveProject(projectPath));
        QCOMPARE(project.isDirty(), false);

        // Removing tile marks dirty
        project.tabModel()->gridModelAt(0)->removeTile(0);
        QCOMPARE(project.isDirty(), true);

        // Saving marks clean
        QVERIFY(project.saveProject(projectPath));
        QCOMPARE(project.isDirty(), false);

        // Cloning tab marks dirty
        int clonedIdx = project.tabModel()->cloneTab(0);
        QCOMPARE(clonedIdx, 1);
        QCOMPARE(project.isDirty(), true);
        QCOMPARE(project.tabModel()->count(), 2);
    }
};

QTEST_MAIN(TestProjectSerialization)
#include "test_project_serialization.moc"
