#ifndef CASTER_PULSE_BACKEND_H
#define CASTER_PULSE_BACKEND_H
#include "AudioBackend.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <functional>

namespace Audio {
struct CommandResult { bool ok; QString output, error; };
using CommandRunner = std::function<CommandResult(const QStringList &)>;
class PulseCompatBackend : public Backend {
public:
    explicit PulseCompatBackend(CommandRunner runner = CommandRunner(),
                                std::function<void(unsigned long)> wait = std::function<void(unsigned long)>(),
                                CommandRunner serviceRunner = CommandRunner());
    Discovery probe() override;
    Discovery enumerateDevices() override;
    Plan validateConfiguration(const Configuration &) override;
    Result apply(const Plan &) override;
    Result stopManagedRouting() override;
    Result status() override;
    Result routePlayback(qint64 processId) override;
    Plan validateRecovery(const QString &endpointId) override;
    Result recoverDevice(const Plan &) override;
    Plan validateServiceRestart() override;
    Result restartAudioServices(const Plan &) override;
    static QVector<Endpoint> decodeDevices(const QJsonArray &, const QJsonArray &, const QJsonArray &);
private:
    struct Module { QString id, name, arguments; };
    CommandRunner runner, serviceRunner;
    std::function<void(unsigned long)> recoveryWait;
    QJsonArray cards, sinks, sources;
    Discovery discovery;
    QString token, boardSink, micSink, previousSource, appliedSource, healthError;
    QVector<Module> owned;
    QMap<QString, QString> loops;
    Configuration effective;
    bool running = false, pipeWireKnown = false;
    QString command(const QStringList &);
    QJsonArray list(const QString &);
    QJsonObject info();
    QString load(const QString &, const QStringList &);
    void unload(const QString &);
    QString resolve(const Endpoint &, const QJsonArray &) const;
    Endpoint endpoint(const QString &) const;
    QString uniqueName(const QString &) const;
    void restoreSource();
};
}
#endif
