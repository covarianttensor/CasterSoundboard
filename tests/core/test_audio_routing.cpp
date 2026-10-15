#include <QTest>
#include <QSignalSpy>
#include <QCoreApplication>

#include "AudioRoutingController.h"
#include "SoundboardProjectController.h"

class TestAudioRouting : public QObject
{
    Q_OBJECT

private slots:
    void testInitialState()
    {
        AudioRoutingController audio;
        QCOMPARE(audio.isRouting(), false);
        QVERIFY(!audio.statusMessage().isEmpty());
        QVERIFY(!audio.diagnosticsLog().isEmpty());

        QStringList inputs = audio.inputDevices();
        QVERIFY(!inputs.isEmpty());
        QVERIFY(inputs.contains(tr("None / Disconnected")));

        QStringList outputs = audio.outputDevices();
        QVERIFY(!outputs.isEmpty());
        QVERIFY(outputs.contains(tr("Default System Output")));

        QStringList recoveries = audio.recoveryDevices();
        QVERIFY(!recoveries.isEmpty());
        QVERIFY(recoveries.contains(tr("Choose a device to enable or reset...")));
    }

    void testConfigurationSetters()
    {
        AudioRoutingController audio;
        QSignalSpy configSpy(&audio, &AudioRoutingController::configurationChanged);

        bool initialMix = audio.mixMicrophone();
        audio.setMixMicrophone(!initialMix);
        QCOMPARE(audio.mixMicrophone(), !initialMix);
        QCOMPARE(configSpy.count(), 1);

        bool initialMon = audio.monitorSoundboard();
        audio.setMonitorSoundboard(!initialMon);
        QCOMPARE(audio.monitorSoundboard(), !initialMon);
        QCOMPARE(configSpy.count(), 2);

        bool initialDef = audio.useAsDefaultInput();
        audio.setUseAsDefaultInput(!initialDef);
        QCOMPARE(audio.useAsDefaultInput(), !initialDef);
        QCOMPARE(configSpy.count(), 3);

        audio.setVirtualMicName("Custom_Podcaster_Mic");
        QCOMPARE(audio.virtualMicName(), QString("Custom_Podcaster_Mic"));
        QCOMPARE(configSpy.count(), 4);

        audio.setSelectedMicrophone("CustomMicLabel");
        QCOMPARE(audio.selectedMicrophone(), QString("CustomMicLabel"));
        QCOMPARE(configSpy.count(), 5);

        audio.setSelectedSystemOutput("CustomSysOut");
        QCOMPARE(audio.selectedSystemOutput(), QString("CustomSysOut"));
        QCOMPARE(configSpy.count(), 6);

        audio.setSelectedMonitorOutput("CustomMonOut");
        QCOMPARE(audio.selectedMonitorOutput(), QString("CustomMonOut"));
        QCOMPARE(configSpy.count(), 7);
    }

    void testRecoveryDeviceSelection()
    {
        AudioRoutingController audio;
        QVERIFY(!audio.recoveryDevices().isEmpty());
        QString initial = audio.selectedRecoveryDevice();
        QCOMPARE(initial, tr("Choose a device to enable or reset..."));

        QSignalSpy recSpy(&audio, &AudioRoutingController::selectedRecoveryDeviceChanged);
        audio.setSelectedRecoveryDevice("Virtual Recovery Device");
        QCOMPARE(audio.selectedRecoveryDevice(), QString("Virtual Recovery Device"));
        QVERIFY(recSpy.count() >= 1);
    }

    void testApplyAndStopSharingSafe()
    {
        SoundboardProjectController project;
        AudioRoutingController audio;
        audio.setProjectController(&project);

        // Call apply, stopSharing, refresh - these should not crash and should update status
        audio.refresh();
        audio.apply();
        audio.stopSharing();

        QVERIFY(!audio.statusMessage().isEmpty());
    }
};

QTEST_MAIN(TestAudioRouting)
#include "test_audio_routing.moc"
