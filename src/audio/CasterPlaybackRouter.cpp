#include "CasterPlaybackRouter.h"
#include <QStandardPaths>
#include <QFile>
namespace Audio {
CasterPlaybackRouter::CasterPlaybackRouter(QObject *parent) : QObject(parent) {
    debounce.setSingleShot(true); debounce.setInterval(30);
    fallback.setInterval(1000);
    connect(&debounce, &QTimer::timeout, this, &CasterPlaybackRouter::streamsChanged);
    connect(&fallback, &QTimer::timeout, this, &CasterPlaybackRouter::streamsChanged);
    connect(&events, &QProcess::readyReadStandardOutput, this, [this] {
        QByteArray data = events.readAllStandardOutput();
        if (enabled && data.contains("sink-input") && !debounce.isActive()) debounce.start();
    });
    connect(&events, &QProcess::readyReadStandardError, this, [this] { events.readAllStandardError(); });
}
CasterPlaybackRouter::~CasterPlaybackRouter() { setEnabled(false); }
void CasterPlaybackRouter::setEnabled(bool value) {
    enabled = value;
    if (value) {
        fallback.start();
        if (events.state() == QProcess::NotRunning) {
            auto path = QStandardPaths::findExecutable("pactl");
            QStringList args = {"subscribe"};
            if (path.isEmpty() && QFile::exists("/.flatpak-info")) {
                const auto flatpakSpawn = QStandardPaths::findExecutable("flatpak-spawn");
                if (!flatpakSpawn.isEmpty()) {
                    path = flatpakSpawn;
                    args.prepend("pactl");
                    args.prepend("--host");
                }
            }
            if (!path.isEmpty()) {
                auto env = QProcessEnvironment::systemEnvironment();
                QString pulseProp = env.value("PULSE_PROP");
                if (pulseProp.isEmpty()) {
                    pulseProp = "media.category=Manager";
                } else if (!pulseProp.contains("media.category")) {
                    pulseProp += " media.category=Manager";
                }
                env.insert("PULSE_PROP", pulseProp);
                events.setProcessEnvironment(env);
                events.start(path, args);
            }
        }
        emit streamsChanged();
    } else {
        fallback.stop(); debounce.stop();
        if (events.state() != QProcess::NotRunning) {
            events.terminate();
            if (!events.waitForFinished(1000)) {
                events.kill();
                events.waitForFinished(1000);
            }
        }
    }
}
}
