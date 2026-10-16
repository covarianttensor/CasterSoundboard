#ifndef AUDIOROUTINGCONTROLLER_H
#define AUDIOROUTINGCONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>

#include "AudioDeviceController.h"

class SoundboardProjectController;

class AudioRoutingController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isRouting READ isRouting NOTIFY routingChanged)
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY busyChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString diagnosticsLog READ diagnosticsLog NOTIFY diagnosticsLogChanged)

    Q_PROPERTY(QStringList inputDevices READ inputDevices NOTIFY devicesChanged)
    Q_PROPERTY(QStringList outputDevices READ outputDevices NOTIFY devicesChanged)
    Q_PROPERTY(QStringList recoveryDevices READ recoveryDevices NOTIFY devicesChanged)

    Q_PROPERTY(QString selectedMicrophone READ selectedMicrophone WRITE setSelectedMicrophone NOTIFY configurationChanged)
    Q_PROPERTY(QString selectedSystemOutput READ selectedSystemOutput WRITE setSelectedSystemOutput NOTIFY configurationChanged)
    Q_PROPERTY(QString selectedMonitorOutput READ selectedMonitorOutput WRITE setSelectedMonitorOutput NOTIFY configurationChanged)
    Q_PROPERTY(QString selectedRecoveryDevice READ selectedRecoveryDevice WRITE setSelectedRecoveryDevice NOTIFY selectedRecoveryDeviceChanged)

    Q_PROPERTY(bool mixMicrophone READ mixMicrophone WRITE setMixMicrophone NOTIFY configurationChanged)
    Q_PROPERTY(bool monitorSoundboard READ monitorSoundboard WRITE setMonitorSoundboard NOTIFY configurationChanged)
    Q_PROPERTY(bool useAsDefaultInput READ useAsDefaultInput WRITE setUseAsDefaultInput NOTIFY configurationChanged)
    Q_PROPERTY(QString virtualMicName READ virtualMicName WRITE setVirtualMicName NOTIFY configurationChanged)

public:
    explicit AudioRoutingController(QObject *parent = nullptr);
    ~AudioRoutingController() override;

    void setProjectController(SoundboardProjectController *project);

    bool isRouting() const;
    void setRoutingActiveForTesting(bool active);
    bool isBusy() const { return m_isBusy; }
    QString statusMessage() const { return m_statusMessage; }
    QString diagnosticsLog() const { return m_diagnosticsLog; }

    QStringList inputDevices() const { return m_inputDeviceNames; }
    QStringList outputDevices() const { return m_outputDeviceNames; }

    QString selectedMicrophone() const { return m_selectedMicrophone; }
    void setSelectedMicrophone(const QString &mic);

    QString selectedSystemOutput() const { return m_selectedSystemOutput; }
    void setSelectedSystemOutput(const QString &out);

    QString selectedMonitorOutput() const { return m_selectedMonitorOutput; }
    void setSelectedMonitorOutput(const QString &mon);

    QString selectedRecoveryDevice() const { return m_selectedRecoveryDevice; }
    void setSelectedRecoveryDevice(const QString &dev);

    QStringList recoveryDevices() const { return m_recoveryDeviceNames; }

    bool mixMicrophone() const { return m_mixMicrophone; }
    void setMixMicrophone(bool mix);

    bool monitorSoundboard() const { return m_monitorSoundboard; }
    void setMonitorSoundboard(bool mon);

    bool useAsDefaultInput() const { return m_useAsDefaultInput; }
    void setUseAsDefaultInput(bool def);

    QString virtualMicName() const { return m_virtualMicName; }
    void setVirtualMicName(const QString &name);

public slots:
    void refresh();
    void apply();
    void stopSharing();
    void recoverSelectedDevice();
    void restartDesktopAudio();

signals:
    void routingChanged();
    void busyChanged();
    void statusMessageChanged();
    void diagnosticsLogChanged();
    void devicesChanged();
    void configurationChanged();
    void selectedRecoveryDeviceChanged();

private slots:
    void onDevicesChanged(const Audio::Discovery &discovery);
    void onPreviewReady(const Audio::Plan &plan);
    void onRecoveryPreviewReady(const Audio::Plan &plan);
    void onRestartPreviewReady(const Audio::Plan &plan);
    void onResultReady(const Audio::Result &result);
    void onPlaybackRoutingChanged(bool active);

private:
    void updateEndpointLists(const QVector<Audio::Endpoint> &endpoints);
    QString endpointIdForLabel(const QString &label, bool isInput) const;
    QString labelForEndpointId(const QString &id, bool isInput) const;

    Audio::AudioDeviceController *m_audioController{nullptr};
    SoundboardProjectController *m_project{nullptr};

    bool m_isBusy{false};
    QString m_statusMessage{"Ready"};
    QString m_diagnosticsLog;

    QStringList m_inputDeviceNames;
    QStringList m_outputDeviceNames;
    QStringList m_recoveryDeviceNames;
    QMap<QString, QString> m_inputLabelToId;
    QMap<QString, QString> m_inputIdToLabel;
    QMap<QString, QString> m_outputLabelToId;
    QMap<QString, QString> m_outputIdToLabel;
    QMap<QString, QString> m_recoveryLabelToId;

    QString m_selectedMicrophone;
    QString m_selectedSystemOutput;
    QString m_selectedMonitorOutput;
    QString m_selectedRecoveryDevice;

    bool m_mixMicrophone{true};
    bool m_monitorSoundboard{true};
    bool m_useAsDefaultInput{false};
    bool m_testingRoutingActive{false};
    QString m_virtualMicName{"Caster Soundboard Mic"};
};

#endif // AUDIOROUTINGCONTROLLER_H
