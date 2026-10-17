#include <QTest>
#include <QSignalSpy>
#include <QJsonObject>
#include <QJsonArray>

#include "ActionProfile.h"
#include "ActionsController.h"
#include "UInputChannel.h"
#include "SoundboardProjectController.h"
#include "SoundTileController.h"

class TestActions : public QObject
{
    Q_OBJECT

private slots:
    void testActionProfileSerialization()
    {
        Actions::Profile p;
        p.id = "test-uuid-123";
        p.name = "Discord PTT";
        p.mode = Actions::ModePushToTalk;
        p.pinnedTabName = "Sound FX";
        p.pttKey = "KEY_V";
        p.muteKey = "KEY_M";
        p.bufferMs = 200;

        QJsonObject json = p.toJson();
        QCOMPARE(json["id"].toString(), QString("test-uuid-123"));
        QCOMPARE(json["name"].toString(), QString("Discord PTT"));
        QCOMPARE(json["mode"].toInt(), static_cast<int>(Actions::ModePushToTalk));
        QCOMPARE(json["pinnedTabName"].toString(), QString("Sound FX"));
        QCOMPARE(json["pttKey"].toString(), QString("KEY_V"));
        QCOMPARE(json["muteKey"].toString(), QString("KEY_M"));
        QCOMPARE(json["bufferMs"].toInt(), 200);

        Actions::Profile restored = Actions::Profile::fromJson(json);
        QCOMPARE(restored.id, p.id);
        QCOMPARE(restored.name, p.name);
        QCOMPARE(restored.mode, p.mode);
        QCOMPARE(restored.pinnedTabName, p.pinnedTabName);
        QCOMPARE(restored.pttKey, p.pttKey);
        QCOMPARE(restored.muteKey, p.muteKey);
        QCOMPARE(restored.bufferMs, p.bufferMs);
    }

    void testActionsControllerCRUD()
    {
        ActionsController controller;
        QCOMPARE(controller.count(), 0);

        QSignalSpy spy(&controller, &ActionsController::profilesChanged);
        int initialRev = controller.revision();

        int idx = controller.addProfile("Stream PTT");
        QCOMPARE(idx, 0);
        QCOMPARE(controller.count(), 1);
        QCOMPARE(spy.count(), 1);
        QVERIFY(controller.revision() > initialRev);

        QVariantMap data = controller.getProfile(0);
        QCOMPARE(data["name"].toString(), QString("Stream PTT"));
        QCOMPARE(data["mode"].toInt(), static_cast<int>(Actions::ModePushToTalk));

        data["bufferMs"] = 180;
        data["pinnedTabName"] = "Tab A";
        controller.setProfile(0, data);
        QCOMPARE(spy.count(), 2);

        QVariantMap updated = controller.getProfile(0);
        QCOMPARE(updated["bufferMs"].toInt(), 180);
        QCOMPARE(updated["pinnedTabName"].toString(), QString("Tab A"));

        QVariantList list = controller.profilesList();
        QCOMPARE(list.size(), 1);

        bool removed = controller.removeProfile(0);
        QVERIFY(removed);
        QCOMPARE(controller.count(), 0);
        QCOMPARE(spy.count(), 3);
    }

    void testTabBindingAndBadges()
    {
        ActionsController controller;
        int idx = controller.addProfile("Podcast Mute");
        QVariantMap data = controller.getProfile(idx);
        data["mode"] = static_cast<int>(Actions::ModeMuteToggle);
        data["muteKey"] = "KEY_M";
        data["pinnedTabName"] = "Jingles";
        controller.setProfile(idx, data);

        QVERIFY(controller.hasActionForTab("Jingles"));
        QCOMPARE(controller.activeBadgeForTab("Jingles"), QString("Mute: M"));
        QVERIFY(!controller.hasActionForTab("Other Tab"));

        // Tab rename synchronization
        controller.handleTabRenamed("Jingles", "Live Drops");
        QVERIFY(!controller.hasActionForTab("Jingles"));
        QVERIFY(controller.hasActionForTab("Live Drops"));
        QCOMPARE(controller.activeBadgeForTab("Live Drops"), QString("Mute: M"));

        // Tab deletion synchronization
        controller.handleTabRemoved("Live Drops");
        QVERIFY(!controller.hasActionForTab("Live Drops"));
    }

    void testProjectSerializationWithActions()
    {
        SoundboardProjectController project;
        project.newProject();

        ActionsController *act = project.actionsController();
        QVERIFY(act != nullptr);

        int idx = act->addProfile("Global PTT");
        QVariantMap pData = act->getProfile(idx);
        pData["pinnedTabName"] = "Main Board";
        pData["pttKey"] = "KEY_SPACE";
        pData["bufferMs"] = 250;
        act->setProfile(idx, pData);

        QJsonObject projJson = project.toJson();
        QVERIFY(projJson.contains("actions"));
        QJsonArray actArray = projJson["actions"].toArray();
        QCOMPARE(actArray.size(), 1);
        QCOMPARE(actArray[0].toObject()["name"].toString(), QString("Global PTT"));
        QCOMPARE(actArray[0].toObject()["pinnedTabName"].toString(), QString("Main Board"));
        QCOMPARE(actArray[0].toObject()["bufferMs"].toInt(), 250);

        // Restore into fresh project
        SoundboardProjectController restoredProject;
        restoredProject.fromJson(projJson);
        ActionsController *restoredAct = restoredProject.actionsController();
        QVERIFY(restoredAct != nullptr);
        QCOMPARE(restoredAct->count(), 1);
        QVariantMap restoredData = restoredAct->getProfile(0);
        QCOMPARE(restoredData["name"].toString(), QString("Global PTT"));
        QCOMPARE(restoredData["pinnedTabName"].toString(), QString("Main Board"));
        QCOMPARE(restoredData["pttKey"].toString(), QString("KEY_SPACE"));
        QCOMPARE(restoredData["bufferMs"].toInt(), 250);
        QVERIFY(restoredAct->hasActionForTab("Main Board"));
    }

    void testKeyNameMapping()
    {
        QCOMPARE(UInputChannel::keyCodeFromName("KEY_V"), 47);
        QCOMPARE(UInputChannel::keyCodeFromName("KEY_SPACE"), 57);
        QCOMPARE(UInputChannel::keyCodeFromName("KEY_M"), 50);
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_V), QString("KEY_V"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_Space), QString("KEY_SPACE"));
    }

    void testPlaybackCoordinationWithZeroBuffer()
    {
        ActionsController controller;
        int idx = controller.addProfile("Zero Buffer PTT");
        QVariantMap data = controller.getProfile(idx);
        data["mode"] = static_cast<int>(Actions::ModePushToTalk);
        data["pttKey"] = "KEY_V";
        data["bufferMs"] = 0;
        data["pinnedTabName"] = "Instant";
        controller.setProfile(idx, data);

        SoundTileController tile1;
        bool callbackExecuted = false;

        controller.coordinatePlayback(&tile1, "Instant", [&]() {
            callbackExecuted = true;
        });

        // With bufferMs == 0, callback should be invoked synchronously/immediately
        QVERIFY(callbackExecuted);

        // Finish tile playback
        controller.coordinatePlaybackFinished(&tile1);
    }

    void testTransmittingState()
    {
        ActionsController controller;
        QCOMPARE(controller.isTransmitting(), false);
        QCOMPARE(controller.activeTransmittingKey(), QString());

        int idx = controller.addProfile("PTT Board");
        QVariantMap data = controller.getProfile(idx);
        data["mode"] = static_cast<int>(Actions::ModePushToTalk);
        data["pttKey"] = "KEY_V";
        data["bufferMs"] = 0;
        data["pinnedTabName"] = "Gaming";
        controller.setProfile(idx, data);

        QSignalSpy spy(&controller, &ActionsController::transmissionChanged);

        SoundTileController tile;
        bool ready = false;
        controller.coordinatePlayback(&tile, "Gaming", [&]() {
            ready = true;
        });

        QVERIFY(ready);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(controller.isTransmitting(), true);
        QCOMPARE(controller.isTransmittingForTab("Gaming"), true);
        QCOMPARE(controller.isTransmittingForTab("Other"), false);
        QCOMPARE(controller.activeTransmittingKey(), QString("KEY_V"));
        QCOMPARE(controller.isTransmittingKey("KEY_V"), true);

        controller.coordinatePlaybackFinished(&tile);
        QCOMPARE(controller.isTransmitting(), false);
        QCOMPARE(controller.isTransmittingForTab("Gaming"), false);
        QCOMPARE(spy.count(), 2);
    }

    void testTestTriggerKey()
    {
        ActionsController controller;
        QSignalSpy spy(&controller, &ActionsController::transmissionChanged);

        controller.testTriggerKey("KEY_F13", 50);
        if (controller.isUInputAvailable()) {
            QCOMPARE(controller.isTransmitting(), true);
            QCOMPARE(controller.activeTransmittingKey(), QString("KEY_F13"));
            QCOMPARE(controller.isTransmittingKey("KEY_F13"), true);
            QCOMPARE(spy.count(), 1);

            // Wait for 50ms test timer
            QTRY_VERIFY_WITH_TIMEOUT(!controller.isTransmitting(), 500);
            QCOMPARE(spy.count(), 2);
        }
    }
};

QTEST_MAIN(TestActions)
#include "test_actions.moc"
