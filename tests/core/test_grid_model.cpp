#include <QTest>
#include <QSignalSpy>
#include "SoundboardGridModel.h"
#include "SoundTileController.h"

class TestGridModel : public QObject
{
    Q_OBJECT

private slots:
    void testEmptyModel()
    {
        SoundboardGridModel model;
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.count(), 0);
        QVERIFY(model.tileAt(0) == nullptr);
    }

    void testAppendAndRemoveTiles()
    {
        SoundboardGridModel model;
        QSignalSpy countSpy(&model, &SoundboardGridModel::countChanged);

        SoundTileController *t1 = model.appendTile();
        QVERIFY(t1 != nullptr);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(countSpy.count(), 1);

        SoundTileController *t2 = model.appendTile();
        QVERIFY(t2 != nullptr);
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(countSpy.count(), 2);

        QCOMPARE(model.tileAt(0), t1);
        QCOMPARE(model.tileAt(1), t2);

        // Remove tile 0
        QVERIFY(model.removeTile(0));
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.tileAt(0), t2);
        QCOMPARE(countSpy.count(), 3);
    }

    void testMoveTile()
    {
        SoundboardGridModel model;
        SoundTileController *t1 = model.appendTile();
        t1->setDisplayName("Tile 1");
        SoundTileController *t2 = model.appendTile();
        t2->setDisplayName("Tile 2");
        SoundTileController *t3 = model.appendTile();
        t3->setDisplayName("Tile 3");

        QCOMPARE(model.tileAt(0)->displayName(), QString("Tile 1"));
        QCOMPARE(model.tileAt(1)->displayName(), QString("Tile 2"));
        QCOMPARE(model.tileAt(2)->displayName(), QString("Tile 3"));

        // Move tile 0 to position 2
        QVERIFY(model.moveTile(0, 2));
        QCOMPARE(model.tileAt(0)->displayName(), QString("Tile 2"));
        QCOMPARE(model.tileAt(1)->displayName(), QString("Tile 3"));
        QCOMPARE(model.tileAt(2)->displayName(), QString("Tile 1"));
    }

    void testModelDataRoles()
    {
        SoundboardGridModel model;
        SoundTileController *t = model.appendTile();
        t->setDisplayName("Laser FX");
        t->setHotkey("Ctrl+1");
        t->setVolume(0.9);
        t->setColor("#9B59B6");

        QModelIndex idx = model.index(0, 0);
        QCOMPARE(model.data(idx, SoundboardGridModel::DisplayNameRole).toString(), QString("Laser FX"));
        QCOMPARE(model.data(idx, SoundboardGridModel::HotkeyRole).toString(), QString("Ctrl+1"));
        QCOMPARE(model.data(idx, SoundboardGridModel::VolumeRole).toReal(), 0.9);
        QCOMPARE(model.data(idx, SoundboardGridModel::ColorRole).toString(), QString("#9B59B6"));
    }

    void testJsonRoundtrip()
    {
        SoundboardGridModel model1;
        SoundTileController *t1 = model1.appendTile();
        t1->setDisplayName("Sound 1");
        t1->setFilePath("/path/1.wav");

        SoundTileController *t2 = model1.appendTile();
        t2->setDisplayName("Sound 2");
        t2->setHotkey("Alt+S");

        QJsonArray arr = model1.toJson();
        QCOMPARE(arr.size(), 2);

        SoundboardGridModel model2;
        QVERIFY(model2.fromJson(arr));
        QCOMPARE(model2.rowCount(), 2);
        QCOMPARE(model2.tileAt(0)->displayName(), QString("Sound 1"));
        QCOMPARE(model2.tileAt(0)->filePath(), QString("/path/1.wav"));
        QCOMPARE(model2.tileAt(1)->displayName(), QString("Sound 2"));
        QCOMPARE(model2.tileAt(1)->hotkey(), QString("Alt+S"));
    }
};

QTEST_MAIN(TestGridModel)
#include "test_grid_model.moc"
