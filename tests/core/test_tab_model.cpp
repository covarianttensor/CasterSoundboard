#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"
#include "SoundTileController.h"

class TestTabModel : public QObject
{
    Q_OBJECT

private slots:
    void testInitialEmpty()
    {
        SoundboardTabModel tabs;
        QCOMPARE(tabs.count(), 0);
        QCOMPARE(tabs.currentIndex(), -1);
        QVERIFY(tabs.currentGridModel() == nullptr);
    }

    void testAddAndSwitchTabs()
    {
        SoundboardTabModel tabs;
        QSignalSpy countSpy(&tabs, &SoundboardTabModel::countChanged);
        QSignalSpy indexSpy(&tabs, &SoundboardTabModel::currentIndexChanged);

        SoundboardGridModel *g1 = tabs.addTab("Show Intros");
        QVERIFY(g1 != nullptr);
        QCOMPARE(tabs.count(), 1);
        QCOMPARE(tabs.currentIndex(), 0);
        QCOMPARE(tabs.tabNameAt(0), QString("Show Intros"));
        QCOMPARE(tabs.currentGridModel(), g1);

        SoundboardGridModel *g2 = tabs.addTab("Sound FX");
        QVERIFY(g2 != nullptr);
        QCOMPARE(tabs.count(), 2);
        QCOMPARE(tabs.tabNameAt(1), QString("Sound FX"));

        tabs.setCurrentIndex(1);
        QCOMPARE(tabs.currentIndex(), 1);
        QCOMPARE(tabs.currentGridModel(), g2);
    }

    void testTabCycling_Issue67()
    {
        SoundboardTabModel tabs;
        tabs.addTab("Tab 1");
        tabs.addTab("Tab 2");
        tabs.addTab("Tab 3");

        QCOMPARE(tabs.currentIndex(), 0);

        // Next tab: 0 -> 1 -> 2 -> 0 (wrapping)
        tabs.nextTab();
        QCOMPARE(tabs.currentIndex(), 1);
        tabs.nextTab();
        QCOMPARE(tabs.currentIndex(), 2);
        tabs.nextTab();
        QCOMPARE(tabs.currentIndex(), 0);

        // Previous tab: 0 -> 2 -> 1 -> 0 (wrapping backwards)
        tabs.previousTab();
        QCOMPARE(tabs.currentIndex(), 2);
        tabs.previousTab();
        QCOMPARE(tabs.currentIndex(), 1);
        tabs.previousTab();
        QCOMPARE(tabs.currentIndex(), 0);
    }

    void testTabRenameAndRemove()
    {
        SoundboardTabModel tabs;
        tabs.addTab("Original Name");
        tabs.addTab("Tab 2");

        QVERIFY(tabs.renameTab(0, "Updated Name"));
        QCOMPARE(tabs.tabNameAt(0), QString("Updated Name"));

        // Removing tab 0
        QVERIFY(tabs.removeTab(0));
        QCOMPARE(tabs.count(), 1);
        QCOMPARE(tabs.tabNameAt(0), QString("Tab 2"));

        // Last remaining tab cannot be removed
        QVERIFY(!tabs.removeTab(0));
        QCOMPARE(tabs.count(), 1);
    }

    void testJsonRoundtrip()
    {
        SoundboardTabModel tabs1;
        SoundboardGridModel *g1 = tabs1.addTab("Music");
        tabs1.setTabHotkey(0, "F1");
        g1->appendTile()->setDisplayName("Theme Song");

        SoundboardGridModel *g2 = tabs1.addTab("Memes");
        tabs1.setTabHotkey(1, "Ctrl+Alt+M");
        g2->appendTile()->setDisplayName("Rimshot");

        QJsonArray arr = tabs1.toJson();
        QCOMPARE(arr.size(), 2);

        SoundboardTabModel tabs2;
        QVERIFY(tabs2.fromJson(arr));
        QCOMPARE(tabs2.count(), 2);
        QCOMPARE(tabs2.tabNameAt(0), QString("Music"));
        QCOMPARE(tabs2.tabHotkeyAt(0), QString("F1"));
        QCOMPARE(tabs2.gridModelAt(0)->count(), 1);
        QCOMPARE(tabs2.gridModelAt(0)->tileAt(0)->displayName(), QString("Theme Song"));

        QCOMPARE(tabs2.tabNameAt(1), QString("Memes"));
        QCOMPARE(tabs2.tabHotkeyAt(1), QString("Ctrl+Alt+M"));
        QCOMPARE(tabs2.gridModelAt(1)->count(), 1);
        QCOMPARE(tabs2.gridModelAt(1)->tileAt(0)->displayName(), QString("Rimshot"));
    }

    void testTabHotkeys()
    {
        SoundboardTabModel tabs;
        tabs.addTab("Tab A");
        tabs.addTab("Tab B");

        QCOMPARE(tabs.tabHotkeyAt(0), QString());
        QCOMPARE(tabs.defaultHotkeyForIndex(0), QString("Alt+1"));
        QCOMPARE(tabs.defaultHotkeyForIndex(1), QString("Alt+2"));

        QSignalSpy dataSpy(&tabs, &QAbstractItemModel::dataChanged);
        QVERIFY(tabs.setTabHotkey(0, "F1"));
        QCOMPARE(tabs.tabHotkeyAt(0), QString("F1"));
        QCOMPARE(dataSpy.count(), 1);

        // Verify model data via HotkeyRole
        QModelIndex idx = tabs.index(0, 0);
        QCOMPARE(tabs.data(idx, SoundboardTabModel::HotkeyRole).toString(), QString("F1"));

        // Clear hotkey
        QVERIFY(tabs.setTabHotkey(0, ""));
        QCOMPARE(tabs.tabHotkeyAt(0), QString());
    }

    void testCloneTab()
    {
        SoundboardTabModel tabs;
        SoundboardGridModel *g1 = tabs.addTab("Original Board");
        tabs.setTabHotkey(0, "Alt+1");

        SoundTileController *t1 = g1->appendTile();
        t1->setDisplayName("Theme Song");
        t1->setFilePath("/sounds/theme.wav");
        t1->setVolume(0.85);
        t1->setIsLooped(true);
        t1->setColor("#E74C3C");
        t1->setTriggerStyle(SoundTileController::PlayStop);
        t1->setHotkey("T");
        t1->setHasCueRegion(true);
        t1->setCueBeginMs(250);
        t1->setCueEndMs(1500);

        SoundTileController *t2 = g1->appendTile();
        t2->setDisplayName("Applause");
        t2->setFilePath("/sounds/applause.wav");
        t2->setVolume(0.5);

        tabs.addTab("Other Board"); // Tab at index 1

        QSignalSpy countSpy(&tabs, &SoundboardTabModel::countChanged);
        QSignalSpy cloneSpy(&tabs, &SoundboardTabModel::tabCloned);
        QSignalSpy indexSpy(&tabs, &SoundboardTabModel::currentIndexChanged);

        // Current index was 0. Clone tab 0
        int clonedIdx = tabs.cloneTab(0);
        QCOMPARE(clonedIdx, 1); // Inserted immediately adjacent
        QCOMPARE(tabs.count(), 3);
        QCOMPARE(countSpy.count(), 1);
        QCOMPARE(cloneSpy.count(), 1);
        QCOMPARE(cloneSpy.at(0).at(0).toString(), QString("Original Board"));
        QCOMPARE(cloneSpy.at(0).at(1).toString(), QString("Original Board (Copy)"));
        QCOMPARE(tabs.currentIndex(), 1); // Automatically switched to cloned tab

        // Names and IDs
        QCOMPARE(tabs.tabNameAt(0), QString("Original Board"));
        QCOMPARE(tabs.tabNameAt(1), QString("Original Board (Copy)"));
        QCOMPARE(tabs.tabNameAt(2), QString("Other Board"));
        QVERIFY(tabs.tabIdAt(0) != tabs.tabIdAt(1));
        QVERIFY(!tabs.tabIdAt(1).isEmpty());

        // Tab hotkey should not conflict
        QCOMPARE(tabs.tabHotkeyAt(1), QString());

        // Deep-copied tiles in the cloned tab
        SoundboardGridModel *clonedGrid = tabs.gridModelAt(1);
        QVERIFY(clonedGrid != nullptr);
        QCOMPARE(clonedGrid->count(), 2);

        SoundTileController *ct1 = clonedGrid->tileAt(0);
        QVERIFY(ct1 != nullptr);
        QCOMPARE(ct1->displayName(), QString("Theme Song"));
        QCOMPARE(ct1->filePath(), QString("/sounds/theme.wav"));
        QCOMPARE(ct1->volume(), 0.85);
        QCOMPARE(ct1->isLooped(), true);
        QCOMPARE(ct1->color(), QString("#E74C3C"));
        QCOMPARE(ct1->triggerStyle(), SoundTileController::PlayStop);
        QCOMPARE(ct1->hotkey(), QString("T"));
        QCOMPARE(ct1->hasCueRegion(), true);
        QCOMPARE(ct1->cueBeginMs(), 250);
        QCOMPARE(ct1->cueEndMs(), 1500);
        // Verify unique UUID
        QVERIFY(ct1->id() != t1->id());
        QVERIFY(!ct1->id().isEmpty());

        SoundTileController *ct2 = clonedGrid->tileAt(1);
        QVERIFY(ct2 != nullptr);
        QCOMPARE(ct2->displayName(), QString("Applause"));
        QCOMPARE(ct2->volume(), 0.5);
        QVERIFY(ct2->id() != t2->id());

        // Clone again from original tab: suffix increment
        int secondCloneIdx = tabs.cloneTab(0);
        QCOMPARE(secondCloneIdx, 1); // inserted adjacent to tab 0
        QCOMPARE(tabs.tabNameAt(1), QString("Original Board (Copy 2)"));
        QCOMPARE(tabs.tabNameAt(2), QString("Original Board (Copy)"));

        // Clone with custom name
        int customCloneIdx = tabs.cloneTab(0, "VIP Board");
        QCOMPARE(customCloneIdx, 1);
        QCOMPARE(tabs.tabNameAt(1), QString("VIP Board"));

        // Invalid index
        QCOMPARE(tabs.cloneTab(-1), -1);
        QCOMPARE(tabs.cloneTab(99), -1);
    }

    void testImportDirectory()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QDir rootDir(tempDir.path());
        rootDir.mkdir("SFX");
        rootDir.mkdir("Music");

        auto touchFile = [](const QString &path) {
            QFile f(path);
            if (f.open(QIODevice::WriteOnly)) {
                f.write("dummy audio content");
                f.close();
            }
        };

        // Create root files
        touchFile(tempDir.filePath("track10.mp3"));
        touchFile(tempDir.filePath("track2.wav"));
        touchFile(tempDir.filePath("track1.ogg"));
        touchFile(tempDir.filePath("readme.txt")); // Non-audio file should be ignored

        // Create nested files
        touchFile(tempDir.filePath("SFX/laser.wav"));
        touchFile(tempDir.filePath("SFX/track1.wav")); // Duplicate base name across folders
        touchFile(tempDir.filePath("Music/theme.flac"));

        SoundboardTabModel tabs;

        // Count checks
        QCOMPARE(tabs.countPlayableFilesInDirectory(tempDir.path(), false), 3); // Root only (track1, track2, track10)
        QCOMPARE(tabs.countPlayableFilesInDirectory(tempDir.path(), true), 6);  // Recursive (3 root + 2 SFX + 1 Music)

        // Invalid directory
        QCOMPARE(tabs.countPlayableFilesInDirectory("/path/does/not/exist", true), 0);
        QCOMPARE(tabs.importDirectory("/path/does/not/exist", true), -1);

        // Empty directory import
        QTemporaryDir emptyDir;
        QVERIFY(emptyDir.isValid());
        QCOMPARE(tabs.importDirectory(emptyDir.path(), true), -1);
        QCOMPARE(tabs.count(), 0);

        // Non-recursive import
        int idx1 = tabs.importDirectory(tempDir.path(), false, "Root Only Board",
                                       SoundboardTabModel::RootFilesFirst,
                                       SoundboardTabModel::FilenameOnly);
        QCOMPARE(idx1, 0);
        QCOMPARE(tabs.count(), 1);
        QCOMPARE(tabs.currentIndex(), 0);
        QCOMPARE(tabs.tabNameAt(0), QString("Root Only Board"));

        SoundboardGridModel *grid1 = tabs.gridModelAt(0);
        QVERIFY(grid1 != nullptr);
        QCOMPARE(grid1->count(), 3);
        // Verify natural numeric sort: track1, track2, track10 (not track1, track10, track2)
        QCOMPARE(grid1->tileAt(0)->displayName(), QString("track1"));
        QCOMPARE(grid1->tileAt(1)->displayName(), QString("track2"));
        QCOMPARE(grid1->tileAt(2)->displayName(), QString("track10"));

        // Recursive import with RootFilesFirst order & DisambiguateDuplicates naming
        int idx2 = tabs.importDirectory(tempDir.path(), true, "Full Library",
                                       SoundboardTabModel::RootFilesFirst,
                                       SoundboardTabModel::DisambiguateDuplicates);
        QCOMPARE(idx2, 1);
        QCOMPARE(tabs.count(), 2);
        QCOMPARE(tabs.currentIndex(), 1);

        SoundboardGridModel *grid2 = tabs.gridModelAt(1);
        QVERIFY(grid2 != nullptr);
        QCOMPARE(grid2->count(), 6);

        // First 3 should be root files in natural order:
        QCOMPARE(grid2->tileAt(0)->displayName(), QString("track1"));
        QCOMPARE(grid2->tileAt(1)->displayName(), QString("track2"));
        QCOMPARE(grid2->tileAt(2)->displayName(), QString("track10"));

        // Followed by nested files:
        // Music/theme.flac (unique, so no prefix)
        // SFX/laser.wav (unique, so no prefix)
        // SFX/track1.wav (collides with root track1, so prefixed as "SFX / track1")
        QCOMPARE(grid2->tileAt(3)->displayName(), QString("theme"));
        QCOMPARE(grid2->tileAt(4)->displayName(), QString("laser"));
        QCOMPARE(grid2->tileAt(5)->displayName(), QString("SFX / track1"));
    }
};

QTEST_MAIN(TestTabModel)
#include "test_tab_model.moc"
