#ifndef CASTER_PLAYBACK_ROUTER_H
#define CASTER_PLAYBACK_ROUTER_H
#include <QObject>
#include <QProcess>
#include <QTimer>
namespace Audio {
// Stream discovery is intentionally independent of player widgets and profiles.
// Event-based movement cannot guarantee that the first audio frame is redirected.
class CasterPlaybackRouter : public QObject {
    Q_OBJECT
public:
    explicit CasterPlaybackRouter(QObject *parent = nullptr);
    ~CasterPlaybackRouter() override;
    void setEnabled(bool);
signals:
    void streamsChanged();
private:
    QProcess events;
    QTimer debounce, fallback;
    bool enabled = false;
};
}
#endif
