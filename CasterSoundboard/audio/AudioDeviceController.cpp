#include "AudioDeviceController.h"
#include <QCoreApplication>
#include <QSettings>
#include <QDebug>
namespace Audio {
AudioWorker::AudioWorker() : backend(createBackend()) {}
void AudioWorker::execute(int operation, Configuration config, Plan plan) {
    Discovery d; Result r;
    switch (operation) {
    case AudioDeviceController::Refresh: d = backend->probe(); break;
    case AudioDeviceController::Preview: plan = backend->validateConfiguration(config); break;
    case AudioDeviceController::Apply: r = backend->apply(plan); if (r.ok) {
            auto routed = backend->routePlayback(QCoreApplication::applicationPid());
            if (!routed.ok) { r.ok = false; r.message = "Routing graph is active, but audio streams could not be redirected. Stop routing and retry.\n" + routed.message; }
        } break;
    case AudioDeviceController::Stop: r = backend->stopManagedRouting(); break;
    case AudioDeviceController::Route: r = backend->routePlayback(QCoreApplication::applicationPid()); break;
    case AudioDeviceController::Status: r = backend->status(); break;
    case AudioDeviceController::RecoveryPreview: plan = backend->validateRecovery(plan.metadata.value("recoveryEndpoint").toString()); break;
    case AudioDeviceController::Recover: r = backend->recoverDevice(plan); d = backend->enumerateDevices(); break;
    case AudioDeviceController::RestartPreview: plan = backend->validateServiceRestart(); break;
    case AudioDeviceController::Restart: r = backend->restartAudioServices(plan); d = backend->enumerateDevices(); break;
    }
    emit completed(operation, d, plan, r);
}
void AudioWorker::shutdown() {
    auto result = backend->stopManagedRouting();
    if (!result.ok) qWarning().noquote() << result.message;
}
AudioDeviceController::AudioDeviceController(QObject *parent) : QObject(parent), worker(new AudioWorker), playback(this) {
    qRegisterMetaType<Configuration>(); qRegisterMetaType<Discovery>(); qRegisterMetaType<Plan>(); qRegisterMetaType<Result>();
    QSettings s("CasterSoundboard", "AudioDevices");
    if (s.value("schema", 1).toInt() == 1 && s.value("backend", "pulse-compat").toString() == "pulse-compat") {
        preferences.microphone = s.value("microphone").toString(); preferences.systemOutput = s.value("systemOutput").toString();
        preferences.monitorOutput = s.value("monitorOutput").toString(); preferences.mix = s.value("mix", true).toBool();
        preferences.monitor = s.value("monitor", true).toBool(); preferences.defaultInput = s.value("defaultInput", false).toBool();
    }
    worker->moveToThread(&thread);
    connect(&thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &AudioDeviceController::execute, worker, &AudioWorker::execute);
    connect(worker, &AudioWorker::completed, this, [this](int op, Discovery d, Plan p, Result r) {
        busy = false; if (op != Route && op != Status) emit busyChanged(false);
        if (op == Refresh) { latest = d; emit devicesChanged(d); }
        else if (op == Preview) emit previewReady(p);
        else if (op == RecoveryPreview) emit recoveryPreviewReady(p);
        else if (op == RestartPreview) emit restartPreviewReady(p);
        else if (op == Recover || op == Restart) {
            if (op == Restart && r.playbackTarget.isEmpty()) { setRouting(false); }
            latest = d; emit devicesChanged(d); emit resultReady(r);
        }
        else if (op == Apply) {
            if (r.ok) { preferences = p.desired; const auto error = save(); if (!error.isEmpty()) r.message += "\n" + error; }
            setRouting(!r.playbackTarget.isEmpty()); emit resultReady(r);
        } else if (op == Stop) { if (r.ok) { setRouting(false); } emit resultReady(r); }
        else if (op == Status || !r.ok) { emit resultReady(r); }
        if (pending >= 0) {
            const int next = pending; const auto config = pendingConfig; const auto plan = pendingPlan; pending = -1;
            request(next, config, plan);
        }
    });
    connect(&playback, &CasterPlaybackRouter::streamsChanged, this, [this] { if (!busy) request(Route); });
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &AudioDeviceController::shutdown);
    thread.start(); QTimer::singleShot(0, this, [this] { request(Refresh); });
}
AudioDeviceController::~AudioDeviceController() { shutdown(); }
void AudioDeviceController::request(int operation, const Configuration &config, const Plan &plan) {
    if (closed) return;
    if (busy) {
        if (operation != Route && operation != Status) { pending = operation; pendingConfig = config; pendingPlan = plan; emit busyChanged(true); }
        return;
    }
    busy = true;
    if (operation != Route && operation != Status) emit busyChanged(true);
    emit execute(operation, config, plan);
}
void AudioDeviceController::shutdown() {
    if (closed) return;
    closed = true; playback.setEnabled(false);
    QMetaObject::invokeMethod(worker, "shutdown", Qt::BlockingQueuedConnection);
    thread.quit(); thread.wait();
}
void AudioDeviceController::setRouting(bool active) {
    const bool changed = routing != active;
    routing = active;
    playback.setEnabled(active);
    if (changed) emit playbackRoutingChanged(active);
}
QString AudioDeviceController::save() {
    QSettings s("CasterSoundboard", "AudioDevices");
    // Do not overwrite a newer schema written by another version.
    if (s.value("schema", 1).toInt() != 1) return "Routing applied, but settings were not saved: unsupported settings schema.";
    s.setValue("schema", 1); s.setValue("backend", "pulse-compat");
    s.remove("application"); // Retire the experimental per-application routing preference.
    s.setValue("microphone", preferences.microphone); s.setValue("systemOutput", preferences.systemOutput);
    s.setValue("monitorOutput", preferences.monitorOutput); s.setValue("mix", preferences.mix);
    s.setValue("monitor", preferences.monitor); s.setValue("defaultInput", preferences.defaultInput); s.sync();
    if (s.status() != QSettings::NoError) return "Routing applied, but audio preferences could not be saved.";
    return {};
}
}
