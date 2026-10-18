#include <QTest>
#include <QSignalSpy>
#include <QJsonObject>
#include <QJsonArray>

#include "PttProfile.h"
#include "PttController.h"
#include "UInputChannel.h"
#include "SoundboardProjectController.h"
#include "SoundTileController.h"

class TestPtt : public QObject
{
    Q_OBJECT

private slots:
    void testPttProfileSerialization()
    {
        Ptt::Profile p;
        p.id = "test-uuid-123";
        p.name = "Discord PTT";
        p.mode = Ptt::ModePushToTalk;
        p.pinnedTabName = "Sound FX";
        p.pttKey = "KEY_V";
        p.muteKey = "KEY_M";
        p.bufferMs = 200;

        QJsonObject json = p.toJson();
        QCOMPARE(json["id"].toString(), QString("test-uuid-123"));
        QCOMPARE(json["name"].toString(), QString("Discord PTT"));
        QCOMPARE(json["mode"].toInt(), static_cast<int>(Ptt::ModePushToTalk));
        QCOMPARE(json["pinnedTabName"].toString(), QString("Sound FX"));
        QCOMPARE(json["pttKey"].toString(), QString("KEY_V"));
        QCOMPARE(json["muteKey"].toString(), QString("KEY_M"));
        QCOMPARE(json["bufferMs"].toInt(), 200);

        Ptt::Profile restored = Ptt::Profile::fromJson(json);
        QCOMPARE(restored.id, p.id);
        QCOMPARE(restored.name, p.name);
        QCOMPARE(restored.mode, p.mode);
        QCOMPARE(restored.pinnedTabName, p.pinnedTabName);
        QCOMPARE(restored.pttKey, p.pttKey);
        QCOMPARE(restored.muteKey, p.muteKey);
        QCOMPARE(restored.bufferMs, p.bufferMs);
    }

    void testPttControllerCRUD()
    {
        PttController controller;
        QCOMPARE(controller.count(), 0);

        QSignalSpy spy(&controller, &PttController::profilesChanged);
        int initialRev = controller.revision();

        int idx = controller.addProfile("Stream PTT");
        QCOMPARE(idx, 0);
        QCOMPARE(controller.count(), 1);
        QCOMPARE(spy.count(), 1);
        QVERIFY(controller.revision() > initialRev);

        QVariantMap data = controller.getProfile(0);
        QCOMPARE(data["name"].toString(), QString("Stream PTT"));
        QCOMPARE(data["mode"].toInt(), static_cast<int>(Ptt::ModePushToTalk));

        data["bufferMs"] = 180;
        data["pinnedTabName"] = "Tab A";
        controller.setProfile(0, data);
        QCOMPARE(spy.count(), 2);

        QVariantMap updated = controller.getProfile(0);
        QCOMPARE(updated["bufferMs"].toInt(), 180);
        QCOMPARE(updated["pinnedTabName"].toString(), QString("Tab A"));

        QVariantList list = controller.profilesList();
        QCOMPARE(list.size(), 1);
        QCOMPARE(controller.property("profiles").toList().size(), 1);

        // Test profile renaming
        QVERIFY(controller.renameProfile(0, "Renamed PTT"));
        QCOMPARE(controller.getProfile(0)["name"].toString(), QString("Renamed PTT"));
        QCOMPARE(spy.count(), 3);
        QVERIFY(!controller.renameProfile(0, "   ")); // Blank name rejected
        QVERIFY(!controller.renameProfile(99, "Invalid")); // Invalid index rejected

        bool removed = controller.removeProfile(0);
        QVERIFY(removed);
        QCOMPARE(controller.count(), 0);
        QCOMPARE(controller.property("profiles").toList().size(), 0);
        QCOMPARE(spy.count(), 4);
    }

    void testTabBindingAndBadges()
    {
        PttController controller;
        int idx = controller.addProfile("Podcast Mute");
        QVariantMap data = controller.getProfile(idx);
        data["mode"] = static_cast<int>(Ptt::ModeMuteToggle);
        data["muteKey"] = "KEY_M";
        data["pinnedTabName"] = "Jingles";
        controller.setProfile(idx, data);

        QVERIFY(controller.hasPttForTab("Jingles"));
        QCOMPARE(controller.activeBadgeForTab("Jingles"), QString("Mute: M"));
        QVERIFY(!controller.hasPttForTab("Other Tab"));

        // Tab cloning synchronization
        controller.handleTabCloned("Jingles", "Jingles (Copy)");
        QCOMPARE(controller.count(), 2);
        QVERIFY(controller.hasPttForTab("Jingles"));
        QVERIFY(controller.hasPttForTab("Jingles (Copy)"));
        QCOMPARE(controller.activeBadgeForTab("Jingles (Copy)"), QString("Mute: M"));
        QCOMPARE(controller.getProfile(1)["name"].toString(), QString("Podcast Mute (Copy)"));
        QVERIFY(controller.getProfile(1)["id"].toString() != controller.getProfile(0)["id"].toString());

        // Tab rename synchronization
        controller.handleTabRenamed("Jingles", "Live Drops");
        QVERIFY(!controller.hasPttForTab("Jingles"));
        QVERIFY(controller.hasPttForTab("Live Drops"));
        QCOMPARE(controller.activeBadgeForTab("Live Drops"), QString("Mute: M"));

        // Tab removal synchronization
        controller.handleTabRemoved("Live Drops");
        QVERIFY(!controller.hasPttForTab("Live Drops"));
    }

    void testProjectSerialization()
    {
        SoundboardProjectController project;
        PttController *act = project.pttController();
        QVERIFY(act != nullptr);

        int idx = act->addProfile("My PTT");
        QVariantMap data = act->getProfile(idx);
        data["mode"] = static_cast<int>(Ptt::ModePushToTalk);
        data["pttKey"] = "KEY_F13";
        data["pinnedTabName"] = "Main";
        data["bufferMs"] = 250;
        act->setProfile(idx, data);

        QJsonObject projJson = project.toJson();
        QVERIFY(projJson.contains("pttProfiles"));
        QJsonArray actArray = projJson["pttProfiles"].toArray();
        QCOMPARE(actArray.size(), 1);
        QCOMPARE(actArray[0].toObject()["pttKey"].toString(), QString("KEY_F13"));
        QCOMPARE(actArray[0].toObject()["bufferMs"].toInt(), 250);

        // Restore into new project
        SoundboardProjectController restoredProject;
        QVERIFY(restoredProject.fromJson(projJson));

        PttController *restoredAct = restoredProject.pttController();
        QVERIFY(restoredAct != nullptr);
        QCOMPARE(restoredAct->count(), 1);
        QVariantMap restoredData = restoredAct->getProfile(0);
        QCOMPARE(restoredData["name"].toString(), QString("My PTT"));
        QCOMPARE(restoredData["pttKey"].toString(), QString("KEY_F13"));
        QCOMPARE(restoredData["bufferMs"].toInt(), 250);
        QCOMPARE(restoredAct->activeBadgeForTab("Main"), QString("PTT: F13"));
    }

    void testTestTriggerKey()
    {
        PttController controller;
        QSignalSpy spy(&controller, &PttController::transmissionChanged);

        controller.testTriggerKey("KEY_V", 150);
        if (controller.isUInputAvailable()) {
            QVERIFY(controller.isTransmitting());
            QCOMPARE(controller.activeTransmittingKey(), QString("KEY_V"));
            QVERIFY(controller.isTransmittingKey("KEY_V"));
            QVERIFY(spy.count() >= 1);

            // Wait for release
            QVERIFY(QTest::qWaitFor([&controller]() {
                return !controller.isTransmitting();
            }, 500));

            QVERIFY(!controller.isTransmitting());
            QVERIFY(controller.activeTransmittingKey().isEmpty());
        }
    }

    void testCoordinatedPlaybackLeadInAndTail()
    {
        PttController controller;
        int idx = controller.addProfile("Lead Tail PTT");
        QVariantMap data = controller.getProfile(idx);
        data["mode"] = static_cast<int>(Ptt::ModePushToTalk);
        data["pttKey"] = "KEY_SPACE";
        data["pinnedTabName"] = "Tab Lead";
        data["bufferMs"] = 100;
        controller.setProfile(idx, data);

        SoundTileController tile;
        bool readyCalled = false;

        QSignalSpy startSpy(&controller, &PttController::pttStarted);
        QSignalSpy endSpy(&controller, &PttController::pttEnded);

        controller.coordinatePlayback(&tile, "Tab Lead", [&readyCalled]() {
            readyCalled = true;
        });

        if (controller.isUInputAvailable()) {
            QVERIFY(controller.isTransmitting());
            QCOMPARE(controller.activeTransmittingKey(), QString("KEY_SPACE"));
            QVERIFY(controller.isTransmittingForTab("Tab Lead"));
            QCOMPARE(startSpy.count(), 1);

            // Ready callback should NOT be called immediately due to 100ms lead-in
            QVERIFY(!readyCalled);

            // Wait for lead-in to expire
            QVERIFY(QTest::qWaitFor([&readyCalled]() {
                return readyCalled;
            }, 300));
            QVERIFY(readyCalled);

            // Sound ends
            controller.coordinatePlaybackFinished(&tile);

            // Tail buffer is 100ms, should still be transmitting right now
            QVERIFY(controller.isTransmitting());

            // Wait for tail release
            QVERIFY(QTest::qWaitFor([&controller]() {
                return !controller.isTransmitting();
            }, 300));

            QVERIFY(!controller.isTransmitting());
            QCOMPARE(endSpy.count(), 1);
        } else {
            // In headless/unprivileged environment, ready callback triggers gracefully
            QVERIFY(readyCalled);
        }
    }

    void testMultipleSoundsOverlap()
    {
        PttController controller;
        int idx = controller.addProfile("Multi Sound PTT");
        QVariantMap data = controller.getProfile(idx);
        data["mode"] = static_cast<int>(Ptt::ModePushToTalk);
        data["pttKey"] = "KEY_V";
        data["pinnedTabName"] = "Tab Multi";
        data["bufferMs"] = 50;
        controller.setProfile(idx, data);

        SoundTileController tile1;
        SoundTileController tile2;

        bool t1Ready = false;
        bool t2Ready = false;

        controller.coordinatePlayback(&tile1, "Tab Multi", [&t1Ready]() { t1Ready = true; });
        controller.coordinatePlayback(&tile2, "Tab Multi", [&t2Ready]() { t2Ready = true; });

        if (controller.isUInputAvailable()) {
            QVERIFY(QTest::qWaitFor([&t1Ready, &t2Ready]() { return t1Ready && t2Ready; }, 200));

            // Finish tile1, tile2 is still playing: session must remain active!
            controller.coordinatePlaybackFinished(&tile1);
            QVERIFY(controller.isTransmitting());

            // Finish tile2: now tail timer starts and eventually releases
            controller.coordinatePlaybackFinished(&tile2);
            QVERIFY(QTest::qWaitFor([&controller]() { return !controller.isTransmitting(); }, 200));
            QVERIFY(!controller.isTransmitting());
        }
    }

    void testStopAll()
    {
        PttController controller;
        int idx = controller.addProfile("StopAll PTT");
        QVariantMap data = controller.getProfile(idx);
        data["mode"] = static_cast<int>(Ptt::ModePushToTalk);
        data["pttKey"] = "KEY_V";
        data["pinnedTabName"] = "Tab Stop";
        data["bufferMs"] = 100;
        controller.setProfile(idx, data);

        SoundTileController tile;
        controller.coordinatePlayback(&tile, "Tab Stop", []() {});

        controller.stopAll();
        QVERIFY(!controller.isTransmitting());
        QVERIFY(controller.activeTransmittingKey().isEmpty());
    }

    void testKeyNameFromEvent_ScanCodeConventions()
    {
        // 1. XKB / Desktop Linux convention (nativeScanCode is evdev + 8)
        // Issue #28 regressions: Period must resolve to KEY_DOT (not KEY_F2), M must resolve to KEY_M (not KEY_CAPSLOCK)
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_Period, 60, UInputChannel::ScanCodeConvention::XkbOffset8), QString("KEY_DOT"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_M, 58, UInputChannel::ScanCodeConvention::XkbOffset8), QString("KEY_M"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_F1, 67, UInputChannel::ScanCodeConvention::XkbOffset8), QString("KEY_F1"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_F2, 68, UInputChannel::ScanCodeConvention::XkbOffset8), QString("KEY_F2"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_CapsLock, 66, UInputChannel::ScanCodeConvention::XkbOffset8), QString("KEY_CAPSLOCK"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_Space, 65, UInputChannel::ScanCodeConvention::XkbOffset8), QString("KEY_SPACE"));

        // Modifiers Left vs Right disambiguation via normalized scan code
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_Control, 37, UInputChannel::ScanCodeConvention::XkbOffset8), QString("KEY_LEFTCTRL"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_Control, 105, UInputChannel::ScanCodeConvention::XkbOffset8), QString("KEY_RIGHTCTRL"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_Shift, 50, UInputChannel::ScanCodeConvention::XkbOffset8), QString("KEY_LEFTSHIFT"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_Shift, 62, UInputChannel::ScanCodeConvention::XkbOffset8), QString("KEY_RIGHTSHIFT"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_Alt, 64, UInputChannel::ScanCodeConvention::XkbOffset8), QString("KEY_LEFTALT"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_Alt, 108, UInputChannel::ScanCodeConvention::XkbOffset8), QString("KEY_RIGHTALT"));

        // 2. Direct Evdev convention (raw kernel code)
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_Period, 52, UInputChannel::ScanCodeConvention::DirectEvdev), QString("KEY_DOT"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_M, 50, UInputChannel::ScanCodeConvention::DirectEvdev), QString("KEY_M"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_F2, 60, UInputChannel::ScanCodeConvention::DirectEvdev), QString("KEY_F2"));

        // 3. Fallback when nativeScanCode is 0 (headless / synthetic events)
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_Period, 0), QString("KEY_DOT"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_M, 0), QString("KEY_M"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_F2, 0), QString("KEY_F2"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_Space, 0), QString("KEY_SPACE"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_Backspace, 0), QString("KEY_BACKSPACE"));
        QCOMPARE(UInputChannel::nameFromQtKey(Qt::Key_Delete, 0), QString("KEY_DELETE"));

        // 4. PttController invocation test
        PttController controller;
        QCOMPARE(controller.keyNameFromEvent(Qt::Key_Period, 60, static_cast<int>(UInputChannel::ScanCodeConvention::XkbOffset8)), QString("KEY_DOT"));
        QCOMPARE(controller.keyNameFromEvent(Qt::Key_M, 58, static_cast<int>(UInputChannel::ScanCodeConvention::XkbOffset8)), QString("KEY_M"));
    }

    void testIsCapturingKeyProperty()
    {
        PttController controller;
        QVERIFY(!controller.isCapturingKey());

        QSignalSpy spy(&controller, &PttController::isCapturingKeyChanged);
        controller.setIsCapturingKey(true);
        QVERIFY(controller.isCapturingKey());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toBool(), true);

        // Setting to same value should not emit redundant signal
        controller.setIsCapturingKey(true);
        QCOMPARE(spy.count(), 0);

        controller.setIsCapturingKey(false);
        QVERIFY(!controller.isCapturingKey());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toBool(), false);
    }
};

QTEST_MAIN(TestPtt)
#include "test_ptt.moc"
