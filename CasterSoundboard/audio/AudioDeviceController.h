#ifndef CASTER_AUDIO_CONTROLLER_H
#define CASTER_AUDIO_CONTROLLER_H
#include "AudioBackend.h"
#include "CasterPlaybackRouter.h"
#include <QObject>
#include <QThread>
#include <QTimer>
namespace Audio {
class AudioWorker : public QObject {
    Q_OBJECT
public:
    AudioWorker();
public slots:
    void execute(int operation, Audio::Configuration config, Audio::Plan plan);
    void shutdown();
signals:
    void completed(int operation, Audio::Discovery discovery, Audio::Plan plan, Audio::Result result);
private:
    std::unique_ptr<Backend> backend;
};
class AudioDeviceController : public QObject {
    Q_OBJECT
public:
    enum Operation { Refresh, Preview, Apply, Stop, Route, Status, RecoveryPreview, Recover, RestartPreview, Restart };
    explicit AudioDeviceController(QObject *parent = nullptr);
    ~AudioDeviceController();
    Configuration saved() const { return preferences; }
    Discovery devices() const { return latest; }
    void request(int operation, const Configuration &config = Configuration(), const Plan &plan = Plan());
    void shutdown();
signals:
    void execute(int operation, Audio::Configuration config, Audio::Plan plan);
    void busyChanged(bool busy);
    void devicesChanged(Audio::Discovery discovery);
    void previewReady(Audio::Plan plan);
    void recoveryPreviewReady(Audio::Plan plan);
    void restartPreviewReady(Audio::Plan plan);
    void resultReady(Audio::Result result);
    void playbackRoutingChanged(bool active);
private:
    QThread thread;
    AudioWorker *worker;
    CasterPlaybackRouter playback;
    Configuration preferences;
    Discovery latest;
    bool busy = false, closed = false, routing = false;
    int pending = -1;
    Configuration pendingConfig;
    Plan pendingPlan;
    QString save();
    void setRouting(bool active);
};
}
#endif
