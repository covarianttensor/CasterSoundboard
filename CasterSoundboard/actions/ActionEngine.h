#pragma once
#include "ActionProfile.h"
#include <QObject>
#include <QMap>
#include <QSet>
#include <functional>
class CasterPlayerWidget;
namespace Actions {
class InputChannel : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual void start(const QString &key) = 0;
    virtual void open(Mode mode) = 0;
    virtual void close() = 0;
signals:
    void ready();
    void opened();
    void closed();
    void failed(QString reason);
};
class Engine : public QObject {
    Q_OBJECT
public:
    explicit Engine(QObject *parent = nullptr, std::function<InputChannel*()> factory = {});
    ~Engine();
    static Engine *instance();
    void start(CasterPlayerWidget *player, const Profile &profile, bool resume);
    void cancel(CasterPlayerWidget *player, bool immediate = true);
    bool contains(CasterPlayerWidget *player) const;
    void shutdown();
signals:
    void error(QString message);
private:
    struct Session;
    struct Run;
    QMap<QString, Session*> sessions;
    QMap<CasterPlayerWidget*, Run*> runs;
    QSet<CasterPlayerWidget*> watched;
    std::function<InputChannel*()> factory;
    quint64 nextRun = 0;
    void arm(CasterPlayerWidget *player);
    void close(Session *session);
    void fail(Session *session, const QString &reason);
};
}
