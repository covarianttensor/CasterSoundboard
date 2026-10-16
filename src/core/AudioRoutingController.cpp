#include "AudioRoutingController.h"
#include "SoundboardProjectController.h"
#include <QRegularExpression>
#include <QDateTime>
#include <QDebug>

AudioRoutingController::AudioRoutingController(QObject *parent)
    : QObject(parent)
{
    m_audioController = new Audio::AudioDeviceController(this);

    connect(m_audioController, &Audio::AudioDeviceController::busyChanged, this, [this](bool b) {
        m_isBusy = b;
        emit busyChanged();
    });

    connect(m_audioController, &Audio::AudioDeviceController::devicesChanged, this, &AudioRoutingController::onDevicesChanged);
    connect(m_audioController, &Audio::AudioDeviceController::previewReady, this, &AudioRoutingController::onPreviewReady);
    connect(m_audioController, &Audio::AudioDeviceController::recoveryPreviewReady, this, &AudioRoutingController::onRecoveryPreviewReady);
    connect(m_audioController, &Audio::AudioDeviceController::restartPreviewReady, this, &AudioRoutingController::onRestartPreviewReady);
    connect(m_audioController, &Audio::AudioDeviceController::resultReady, this, &AudioRoutingController::onResultReady);
    connect(m_audioController, &Audio::AudioDeviceController::playbackRoutingChanged, this, &AudioRoutingController::onPlaybackRoutingChanged);

    // Load saved preferences
    Audio::Configuration prefs = m_audioController->saved();
    m_mixMicrophone = prefs.mix;
    m_monitorSoundboard = prefs.monitor;
    m_useAsDefaultInput = prefs.defaultInput;
    if (!prefs.virtualMicName.isEmpty()) {
        m_virtualMicName = QString(prefs.virtualMicName).replace('_', ' ');
    }

    m_diagnosticsLog = tr("[%1] Audio Devices controller initialized.\n")
                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss"));

    updateEndpointLists({});
    refresh();
}

AudioRoutingController::~AudioRoutingController()
{
}

void AudioRoutingController::setProjectController(SoundboardProjectController *project)
{
    m_project = project;
}

bool AudioRoutingController::isRouting() const
{
    if (m_testingRoutingActive)
        return true;
    return m_audioController && m_audioController->isRouting();
}

void AudioRoutingController::setRoutingActiveForTesting(bool active)
{
    if (m_testingRoutingActive == active)
        return;
    m_testingRoutingActive = active;
    emit routingChanged();
}

void AudioRoutingController::setSelectedMicrophone(const QString &mic)
{
    if (m_selectedMicrophone == mic)
        return;
    m_selectedMicrophone = mic;
    emit configurationChanged();
}

void AudioRoutingController::setSelectedSystemOutput(const QString &out)
{
    if (m_selectedSystemOutput == out)
        return;
    m_selectedSystemOutput = out;
    emit configurationChanged();
}

void AudioRoutingController::setSelectedMonitorOutput(const QString &mon)
{
    if (m_selectedMonitorOutput == mon)
        return;
    m_selectedMonitorOutput = mon;
    emit configurationChanged();
}

void AudioRoutingController::setMixMicrophone(bool mix)
{
    if (m_mixMicrophone == mix)
        return;
    m_mixMicrophone = mix;
    emit configurationChanged();
}

void AudioRoutingController::setMonitorSoundboard(bool mon)
{
    if (m_monitorSoundboard == mon)
        return;
    m_monitorSoundboard = mon;
    emit configurationChanged();
}

void AudioRoutingController::setUseAsDefaultInput(bool def)
{
    if (m_useAsDefaultInput == def)
        return;
    m_useAsDefaultInput = def;
    emit configurationChanged();
}

void AudioRoutingController::setVirtualMicName(const QString &name)
{
    if (m_virtualMicName == name)
        return;
    m_virtualMicName = name;
    emit configurationChanged();
}

void AudioRoutingController::setSelectedRecoveryDevice(const QString &dev)
{
    if (m_selectedRecoveryDevice == dev)
        return;
    m_selectedRecoveryDevice = dev;
    emit selectedRecoveryDeviceChanged();
}

void AudioRoutingController::refresh()
{
    if (!m_audioController)
        return;
    m_statusMessage = tr("Scanning audio devices...");
    emit statusMessageChanged();
    m_audioController->request(Audio::AudioDeviceController::Refresh);
}

void AudioRoutingController::apply()
{
    if (!m_audioController)
        return;

    m_lastErrorMessage.clear();
    emit errorChanged();

    Audio::Configuration config;
    config.microphone = endpointIdForLabel(m_selectedMicrophone, true);
    config.systemOutput = endpointIdForLabel(m_selectedSystemOutput, false);
    config.monitorOutput = endpointIdForLabel(m_selectedMonitorOutput, false);

    // If no microphone is selected, automatically disable mixing so validation succeeds
    if (config.microphone.isEmpty()) {
        config.mix = false;
    } else {
        config.mix = m_mixMicrophone;
    }

    // If monitor output is empty, fallback to system output
    if (config.monitorOutput.isEmpty()) {
        config.monitorOutput = config.systemOutput;
    }
    config.monitor = m_monitorSoundboard;
    config.defaultInput = m_useAsDefaultInput;

    QString sanitized = m_virtualMicName.trimmed();
    sanitized.replace(QRegularExpression("[^A-Za-z0-9_ -]"), "");
    sanitized.replace(' ', '_');
    if (sanitized.isEmpty()) {
        sanitized = "Virtual_Microphone";
    }
    config.virtualMicName = sanitized;

    m_statusMessage = tr("Validating routing plan...");
    emit statusMessageChanged();

    m_audioController->request(Audio::AudioDeviceController::Preview, config);
}

void AudioRoutingController::stopSharing()
{
    if (!m_audioController)
        return;
    m_statusMessage = tr("Stopping virtual audio routing...");
    emit statusMessageChanged();
    m_audioController->request(Audio::AudioDeviceController::Stop);
}

void AudioRoutingController::recoverSelectedDevice()
{
    if (!m_audioController)
        return;
    QString id = m_recoveryLabelToId.value(m_selectedRecoveryDevice);
    if (id.isEmpty())
        return;
    Audio::Plan p;
    p.metadata["recoveryEndpoint"] = id;
    m_statusMessage = tr("Checking recovery plan...");
    emit statusMessageChanged();
    m_audioController->request(Audio::AudioDeviceController::RecoveryPreview, Audio::Configuration(), p);
}

void AudioRoutingController::restartDesktopAudio()
{
    if (!m_audioController)
        return;
    m_statusMessage = tr("Validating audio service restart...");
    emit statusMessageChanged();
    m_audioController->request(Audio::AudioDeviceController::RestartPreview);
}

void AudioRoutingController::onDevicesChanged(const Audio::Discovery &discovery)
{
    updateEndpointLists(discovery.endpoints);

    QString logEntry = tr("[%1] Probed backend: %2 | Status: %3\n")
                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                           .arg(discovery.backend.isEmpty() ? "Unknown" : discovery.backend)
                           .arg(discovery.status.isEmpty() ? "Ready" : discovery.status);
    m_diagnosticsLog.prepend(logEntry);
    emit diagnosticsLogChanged();

    m_statusMessage = discovery.status.isEmpty() ? tr("Devices updated") : discovery.status;
    emit statusMessageChanged();
}

void AudioRoutingController::onPreviewReady(const Audio::Plan &plan)
{
    if (!plan.error.isEmpty()) {
        m_statusMessage = tr("Validation error: %1").arg(plan.error);
        m_lastErrorMessage = plan.error;
        m_diagnosticsLog.prepend(tr("[%1] Validation error: %2\n")
                                     .arg(QDateTime::currentDateTime().toString("hh:mm:ss"), plan.error));
        emit statusMessageChanged();
        emit diagnosticsLogChanged();
        emit errorChanged();
        return;
    }

    m_lastErrorMessage.clear();
    emit errorChanged();
    m_statusMessage = tr("Applying virtual audio routing...");
    emit statusMessageChanged();

    m_audioController->request(Audio::AudioDeviceController::Apply, plan.desired, plan);
}

void AudioRoutingController::onRecoveryPreviewReady(const Audio::Plan &plan)
{
    if (!plan.error.isEmpty()) {
        m_statusMessage = tr("Recovery error: %1").arg(plan.error);
        m_diagnosticsLog.prepend(tr("[%1] Recovery error: %2\n")
                                     .arg(QDateTime::currentDateTime().toString("hh:mm:ss"), plan.error));
        emit statusMessageChanged();
        emit diagnosticsLogChanged();
        return;
    }

    m_diagnosticsLog.prepend(tr("[%1] Planned device recovery:\n%2\n")
                                 .arg(QDateTime::currentDateTime().toString("hh:mm:ss"), plan.changes.join('\n')));
    m_statusMessage = tr("Recovering audio device...");
    emit statusMessageChanged();
    emit diagnosticsLogChanged();

    m_audioController->request(Audio::AudioDeviceController::Recover, Audio::Configuration(), plan);
}

void AudioRoutingController::onRestartPreviewReady(const Audio::Plan &plan)
{
    if (!plan.error.isEmpty()) {
        m_statusMessage = tr("Service restart error: %1").arg(plan.error);
        m_diagnosticsLog.prepend(tr("[%1] Service restart error: %2\n")
                                     .arg(QDateTime::currentDateTime().toString("hh:mm:ss"), plan.error));
        emit statusMessageChanged();
        emit diagnosticsLogChanged();
        return;
    }

    m_diagnosticsLog.prepend(tr("[%1] Planned audio restart:\n%2\n")
                                 .arg(QDateTime::currentDateTime().toString("hh:mm:ss"), plan.changes.join('\n')));
    m_statusMessage = tr("Restarting desktop audio service...");
    emit statusMessageChanged();
    emit diagnosticsLogChanged();

    m_audioController->request(Audio::AudioDeviceController::Restart, Audio::Configuration(), plan);
}

void AudioRoutingController::onResultReady(const Audio::Result &result)
{
    m_statusMessage = result.message;
    m_diagnosticsLog.prepend(tr("[%1] Result (%2): %3\n")
                                 .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                                 .arg(result.ok ? "OK" : "ERROR")
                                 .arg(result.message));
    if (!result.ok) {
        m_lastErrorMessage = result.message;
        emit errorChanged();
    } else {
        m_lastErrorMessage.clear();
        emit errorChanged();
    }
    emit statusMessageChanged();
    emit diagnosticsLogChanged();
    emit routingChanged();
}

void AudioRoutingController::onPlaybackRoutingChanged(bool active)
{
    emit routingChanged();
    if (m_project) {
        if (active && !m_audioController->playbackSinkName().isEmpty()) {
            m_project->setOutputDeviceName(m_audioController->playbackSinkName());
        } else {
            m_project->setOutputDeviceName(tr("Default System Output"));
        }
        m_project->refreshAudioDevices();
    }
}

void AudioRoutingController::updateEndpointLists(const QVector<Audio::Endpoint> &endpoints)
{
    m_inputDeviceNames.clear();
    m_outputDeviceNames.clear();
    m_recoveryDeviceNames.clear();
    m_inputLabelToId.clear();
    m_inputIdToLabel.clear();
    m_outputLabelToId.clear();
    m_outputIdToLabel.clear();
    m_recoveryLabelToId.clear();

    m_defaultOutputId.clear();
    m_defaultInputId.clear();

    // Identify active and available default endpoints
    for (const auto &ep : endpoints) {
        if (!ep.input && ep.available) {
            if (ep.active && m_defaultOutputId.isEmpty()) {
                m_defaultOutputId = ep.id;
            } else if (m_defaultOutputId.isEmpty()) {
                m_defaultOutputId = ep.id;
            }
        }
        if (ep.input && ep.available) {
            if (ep.active && m_defaultInputId.isEmpty()) {
                m_defaultInputId = ep.id;
            } else if (m_defaultInputId.isEmpty()) {
                m_defaultInputId = ep.id;
            }
        }
    }

    m_inputDeviceNames.append(tr("None / Disconnected"));
    m_inputLabelToId.insert(tr("None / Disconnected"), QString());

    m_outputDeviceNames.append(tr("Default System Output"));
    m_outputLabelToId.insert(tr("Default System Output"), m_defaultOutputId);

    m_recoveryDeviceNames.append(tr("Choose a device to enable or reset..."));
    m_recoveryLabelToId.insert(tr("Choose a device to enable or reset..."), QString());

    for (const auto &ep : endpoints) {
        QString label = ep.label.isEmpty() ? ep.id : ep.label;
        if (ep.input) {
            if (!m_inputDeviceNames.contains(label)) {
                m_inputDeviceNames.append(label);
                m_inputLabelToId.insert(label, ep.id);
                m_inputIdToLabel.insert(ep.id, label);
            }
        } else {
            if (!m_outputDeviceNames.contains(label)) {
                m_outputDeviceNames.append(label);
                m_outputLabelToId.insert(label, ep.id);
                m_outputIdToLabel.insert(ep.id, label);
            }
        }

        if (!m_recoveryDeviceNames.contains(label)) {
            m_recoveryDeviceNames.append(label);
            m_recoveryLabelToId.insert(label, ep.id);
        }
    }

    if (m_selectedRecoveryDevice.isEmpty() || !m_recoveryDeviceNames.contains(m_selectedRecoveryDevice)) {
        m_selectedRecoveryDevice = m_recoveryDeviceNames.first();
        emit selectedRecoveryDeviceChanged();
    }

    Audio::Configuration prefs = m_audioController->saved();
    if (!prefs.microphone.isEmpty() && m_inputIdToLabel.contains(prefs.microphone)) {
        m_selectedMicrophone = m_inputIdToLabel.value(prefs.microphone);
    } else if (m_selectedMicrophone.isEmpty() && m_inputDeviceNames.size() > 1) {
        m_selectedMicrophone = m_inputDeviceNames.at(1);
    }

    if (!prefs.systemOutput.isEmpty() && m_outputIdToLabel.contains(prefs.systemOutput)) {
        m_selectedSystemOutput = m_outputIdToLabel.value(prefs.systemOutput);
    } else if (m_selectedSystemOutput.isEmpty()) {
        m_selectedSystemOutput = m_outputDeviceNames.first();
    }

    if (!prefs.monitorOutput.isEmpty() && m_outputIdToLabel.contains(prefs.monitorOutput)) {
        m_selectedMonitorOutput = m_outputIdToLabel.value(prefs.monitorOutput);
    } else if (m_selectedMonitorOutput.isEmpty() && m_outputDeviceNames.size() > 1) {
        m_selectedMonitorOutput = m_outputDeviceNames.at(1);
    }

    emit devicesChanged();
    emit configurationChanged();
}

QString AudioRoutingController::endpointIdForLabel(const QString &label, bool isInput) const
{
    if (label.isEmpty() || label == tr("Default System Output") || label == "Default System Output") {
        return isInput ? QString() : m_defaultOutputId;
    }
    if (label == tr("None / Disconnected") || label == "None / Disconnected") {
        return QString();
    }

    const auto &map = isInput ? m_inputLabelToId : m_outputLabelToId;
    if (map.contains(label) && !map.value(label).isEmpty()) {
        return map.value(label);
    }

    // Fuzzy matching against Qt Multimedia descriptions or device names
    const QString cleanLabel = label.toLower();
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        if (it.value().isEmpty())
            continue;
        const QString keyLower = it.key().toLower();
        if (keyLower.contains(cleanLabel) || cleanLabel.contains(keyLower)) {
            return it.value();
        }
        const QStringList words = cleanLabel.split(QRegularExpression("[^a-z0-9]+"), Qt::SkipEmptyParts);
        for (const QString &w : words) {
            if (w.length() >= 4 && (keyLower.contains(w) || it.value().toLower().contains(w))) {
                return it.value();
            }
        }
    }

    return isInput ? QString() : m_defaultOutputId;
}

QString AudioRoutingController::labelForEndpointId(const QString &id, bool isInput) const
{
    if (isInput) {
        return m_inputIdToLabel.value(id, tr("None / Disconnected"));
    } else {
        return m_outputIdToLabel.value(id, tr("Default System Output"));
    }
}
