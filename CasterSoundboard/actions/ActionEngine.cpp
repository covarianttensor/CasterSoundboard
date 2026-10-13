#include "ActionEngine.h"
#include "../CasterPlayer.h"
#include <QApplication>
#include <QFile>
#include <QTemporaryFile>
#include <QProcess>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QPointer>

namespace Actions {
class HelperChannel : public InputChannel {
public:
    QProcess process;
    QTemporaryFile script;
    QTimer heartbeat, startup;
    QByteArray pending;
    bool stopped = false;
    HelperChannel() {
        heartbeat.setInterval(1000);
        startup.setSingleShot(true); startup.setInterval(7000);
        connect(&heartbeat, &QTimer::timeout, this, [this] { send({{"op", "ping"}}); });
        connect(&startup, &QTimer::timeout, this, [this] { emit failed("Keyboard helper timed out."); });
        connect(&process, &QProcess::readyReadStandardOutput, this, [this] {
            pending += process.readAllStandardOutput();
            while (pending.contains('\n')) {
                int end = pending.indexOf('\n');
                auto event = QJsonDocument::fromJson(pending.left(end)).object().value("event").toString();
                pending.remove(0, end + 1);
                if (event == "ready") { startup.stop(); emit ready(); }
                else if (event == "opened") { startup.stop(); emit opened(); }
                else if (event == "closed") { startup.stop(); emit closed(); }
                else emit failed("Invalid keyboard helper response.");
            }
        });
        connect(&process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this] {
            heartbeat.stop(); startup.stop();
            if (!stopped) emit failed("Keyboard helper stopped: " + QString::fromUtf8(process.readAllStandardError()).trimmed());
        });
        connect(&process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
            if (!stopped) emit failed("Keyboard helper: " + process.errorString());
        });
    }
    ~HelperChannel() override {
        stopped = true; heartbeat.stop(); startup.stop();
        process.closeWriteChannel();
        if (process.state() != QProcess::NotRunning && !process.waitForFinished(1000)) {
            process.terminate();
            if (!process.waitForFinished(1000)) { process.kill(); process.waitForFinished(1000); }
        }
    }
    void send(const QJsonObject &object) {
        if (process.write(QJsonDocument(object).toJson(QJsonDocument::Compact) + '\n') < 0)
            emit failed("Cannot communicate with keyboard helper.");
    }
    void start(const QString &key) override {
        QFile resource(":/actions/input_helper.py");
        const auto python = QStandardPaths::findExecutable("python3");
        if (python.isEmpty() || !resource.open(QIODevice::ReadOnly) || !script.open()) {
            emit failed("Actions need Python 3, libevdev and access to /dev/uinput."); return;
        }
        const auto source = resource.readAll();
        if (script.write(source) != source.size() || !script.flush()) {
            emit failed("Cannot prepare keyboard helper."); return;
        }
        process.start(python, {"-B", script.fileName(), key});
        heartbeat.start(); startup.start();
    }
    void open(Mode mode) override { startup.start(); send({{"op", "open"}, {"mode", mode == PTT ? "ptt" : "mute"}}); }
    void close() override { startup.start(); send({{"op", "close"}}); }
};
struct Engine::Session {
    enum State { Starting, Idle, Opening, Active, Closing, Failed } state = Starting;
    InputChannel *channel = nullptr;
    Mode mode = None;
    int bufferMs = 0;
    QSet<CasterPlayerWidget*> users;
    QTimer tail, maximum;
};
struct Engine::Run {
    quint64 id;
    Session *session;
    Profile profile;
    bool resume = false, started = false;
    QTimer lead;
};
Engine::Engine(QObject *parent, std::function<InputChannel*()> make) : QObject(parent), factory(make) {
    if (!factory) factory = [] { return new HelperChannel; };
}
Engine::~Engine() { shutdown(); }
Engine *Engine::instance() {
    static QPointer<Engine> engine;
    if (!engine) {
        engine = new Engine(qApp);
        QObject::connect(qApp, &QCoreApplication::aboutToQuit, engine, &Engine::shutdown);
    }
    return engine;
}
bool Engine::contains(CasterPlayerWidget *p) const { return runs.contains(p); }
void Engine::start(CasterPlayerWidget *p, const Profile &profile, bool resume) {
    if (contains(p)) return;
    p->actionError.clear();
    if (profile.mode == None || !profile.attached) { p->startPlayback(resume); return; }
    if (!valid(profile)) { p->actionError = "Invalid Actions profile."; emit error(p->actionError); return; }
    Session *s = sessions.value(profile.inputKey());
    if (!s || s->state == Session::Failed) {
        if (s) { delete s->channel; delete s; }
        s = new Session;
        sessions[profile.inputKey()] = s;
        s->channel = factory(); s->channel->setParent(this);
        s->tail.setSingleShot(true); s->maximum.setSingleShot(true);
        connect(&s->tail, &QTimer::timeout, this, [this,s] { close(s); });
        connect(&s->maximum, &QTimer::timeout, this, [this,s] { fail(s, "Action stopped after the five-minute limit."); });
        connect(s->channel, &InputChannel::failed, this, [this,s](QString reason) { fail(s, reason); });
        connect(s->channel, &InputChannel::ready, this, [this,s] {
            s->state = Session::Idle;
            if (!s->users.isEmpty()) { s->state = Session::Opening; s->channel->open(s->mode); }
        });
        connect(s->channel, &InputChannel::opened, this, [this,s] {
            s->state = Session::Active; s->maximum.start(300000);
            if (s->users.isEmpty()) close(s);
            else for (auto p : s->users) arm(p);
        });
        connect(s->channel, &InputChannel::closed, this, [s] { s->state = Session::Idle; s->maximum.stop(); });
    }
    if (s->state == Session::Closing || (!s->users.isEmpty() && s->mode != profile.mode) ||
        ((s->state == Session::Active || s->state == Session::Opening) && s->mode != profile.mode)) {
        p->actionError = "This key is finishing or is in use by a different talk mode. Try again.";
        emit error(p->actionError); return;
    }
    if (s->state == Session::Idle) s->bufferMs = 0;
    auto r = new Run;
    r->id = ++nextRun;
    r->session = s; r->profile = profile; r->resume = resume; r->lead.setSingleShot(true);
    runs[p] = r; s->users.insert(p); s->tail.stop(); s->mode = profile.mode;
    s->bufferMs = qMax(s->bufferMs, profile.bufferMs);
    p->actionPending = true;
    if (!watched.contains(p)) {
        watched.insert(p);
        connect(p, &QObject::destroyed, this, [this,p] {
            auto run = runs.take(p);
            if (run) {
                auto session = run->session;
                session->users.remove(p); delete run;
                if (session->users.isEmpty()) close(session);
            }
            watched.remove(p);
        });
        connect(p->player, &QMediaPlayer::playbackStateChanged, this, [this,p](QMediaPlayer::PlaybackState) {
            // Allow the player's existing full-track loop handler to restart first.
            const auto id = runs.contains(p) ? runs[p]->id : 0;
            QTimer::singleShot(0, this, [this,p,id] {
                if (runs.contains(p) && runs[p]->id == id && runs[p]->started && p->player->state() != QMediaPlayer::PlayingState)
                    cancel(p, p->player->state() == QMediaPlayer::PausedState);
            });
        });
        connect(p->player, &QMediaPlayer::errorOccurred, this, [this,p](QMediaPlayer::Error) {
            if (runs.contains(p)) { p->actionError = p->player->errorString(); cancel(p); emit error(p->actionError); }
        });
    }
    connect(&r->lead, &QTimer::timeout, this, [this,p,r] {
        if (runs.value(p) != r) return;
        const auto id = r->id;
        r->started = true; p->actionPending = false; p->startPlayback(r->resume);
        QTimer::singleShot(5000, this, [this,p,id] {
            if (runs.contains(p) && runs[p]->id == id && p->player->state() != QMediaPlayer::PlayingState) {
                p->actionError = "Sound did not start."; cancel(p); emit error(p->actionError);
            }
        });
    });
    if (s->state == Session::Starting) {
        if (s->users.size() == 1) s->channel->start(profile.inputKey());
    } else if (s->state == Session::Idle) { s->state = Session::Opening; s->channel->open(s->mode); }
    else if (s->state == Session::Active) arm(p);
}
void Engine::arm(CasterPlayerWidget *p) {
    if (runs.contains(p) && !runs[p]->started && !runs[p]->lead.isActive())
        runs[p]->lead.start(runs[p]->profile.bufferMs);
}
void Engine::cancel(CasterPlayerWidget *p, bool immediate) {
    auto r = runs.take(p); if (!r) return;
    auto s = r->session;
    p->actionPending = false;
    s->users.remove(p); delete r;
    if (s->users.isEmpty()) {
        if (immediate) close(s);
        else s->tail.start(s->bufferMs);
    }
}
void Engine::close(Session *s) {
    if (!s->users.isEmpty() || s->state != Session::Active) return;
    s->tail.stop(); s->state = Session::Closing; s->channel->close();
}
void Engine::fail(Session *s, const QString &reason) {
    if (s->state == Session::Failed) return;
    s->state = Session::Failed; s->tail.stop(); s->maximum.stop();
    auto users = s->users;
    for (auto p : users) {
        p->actionError = reason; cancel(p); p->player->stop();
    }
    // EOF closes the helper's PTT/mute scope, even when opening failed.
    auto channel = s->channel; s->channel = nullptr;
    if (channel) { channel->disconnect(this); channel->deleteLater(); }
    emit error(reason);
}
void Engine::shutdown() {
    for (auto p : runs.keys()) { cancel(p); p->player->stop(); }
    for (auto s : sessions) { delete s->channel; delete s; }
    sessions.clear();
}
}
