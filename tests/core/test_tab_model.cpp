#include <QTest>
#include <QSignalSpy>
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
};

QTEST_MAIN(TestTabModel)
#include "test_tab_model.moc"
