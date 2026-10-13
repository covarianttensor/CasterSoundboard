#ifndef CASTER_AUDIO_BACKEND_H
#define CASTER_AUDIO_BACKEND_H
#include <QString>
#include <QStringList>
#include <QVector>
#include <QVariantMap>
#include <memory>

namespace Audio {
struct Endpoint {
    QString id, label;
    bool input = false, available = true, active = false;
    bool recoverySupported = false;
    QString healthWarning;
    QVariantMap metadata;
};
struct Configuration {
    QString microphone, systemOutput, monitorOutput;
    bool mix = true, monitor = true, defaultInput = false;
    bool operator==(const Configuration &o) const {
        return microphone == o.microphone && systemOutput == o.systemOutput &&
            monitorOutput == o.monitorOutput && mix == o.mix && monitor == o.monitor && defaultInput == o.defaultInput;
    }
};
struct Capabilities {
    Capabilities(bool p = false, bool v = false, bool l = false, bool d = false, bool s = false, bool h = false, bool restart = false)
        : profiles(p), virtualSinks(v), loopbacks(l), defaults(d), streamRetargeting(s), hotplug(h), serviceRestart(restart) {}
    bool profiles, virtualSinks, loopbacks;
    bool defaults, streamRetargeting, hotplug, serviceRestart;
};
struct Discovery {
    QString backend, status, error;
    Capabilities capabilities;
    QVector<Endpoint> endpoints;
};
struct Plan {
    Configuration desired;
    QStringList changes;
    QString error;
    QVariantMap metadata;
};
struct Result {
    Result(bool success = false, QString text = QString(), QString target = QString(), QString source = QString())
        : ok(success), message(text), playbackTarget(target), voiceSource(source) {}
    bool ok;
    QString message, playbackTarget, voiceSource;
};
// All provider operations run on one worker thread. Models have no platform fields.
class Backend {
public:
    virtual ~Backend() {}
    virtual Discovery probe() = 0;
    virtual Discovery enumerateDevices() = 0;
    virtual Plan validateConfiguration(const Configuration &) = 0;
    virtual Result apply(const Plan &) = 0;
    virtual Result stopManagedRouting() = 0;
    virtual Result status() = 0;
    virtual Result routePlayback(qint64 processId) = 0;
    virtual Plan validateRecovery(const QString &) {
        Plan p; p.error = "Device recovery is not supported by this backend."; return p;
    }
    virtual Result recoverDevice(const Plan &) { return {false, "Device recovery is not supported by this backend."}; }
    virtual Plan validateServiceRestart() { Plan p; p.error = "Service restart is not supported by this backend."; return p; }
    virtual Result restartAudioServices(const Plan &) { return {false, "Service restart is not supported by this backend."}; }
};
std::unique_ptr<Backend> createBackend();
}
Q_DECLARE_METATYPE(Audio::Configuration)
Q_DECLARE_METATYPE(Audio::Discovery)
Q_DECLARE_METATYPE(Audio::Plan)
Q_DECLARE_METATYPE(Audio::Result)
#endif
