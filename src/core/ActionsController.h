#ifndef ACTIONSCONTROLLER_H
#define ACTIONSCONTROLLER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QSet>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QJsonArray>
#include <functional>

#include "ActionProfile.h"

class UInputChannel;
class SoundTileController;

class ActionsController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY profilesChanged)
    Q_PROPERTY(int revision READ revision NOTIFY profilesChanged)
    Q_PROPERTY(QVariantList profiles READ profilesList NOTIFY profilesChanged)
    Q_PROPERTY(bool uinputAvailable READ isUInputAvailable NOTIFY uinputStatusChanged)
    Q_PROPERTY(QString uinputStatus READ uinputStatusMessage NOTIFY uinputStatusChanged)
    Q_PROPERTY(bool isTransmitting READ isTransmitting NOTIFY transmissionChanged)
    Q_PROPERTY(QString activeTransmittingKey READ activeTransmittingKey NOTIFY transmissionChanged)

public:
    explicit ActionsController(QObject *parent = nullptr);
    ~ActionsController() override;

    int count() const { return m_profiles.size(); }
    int revision() const { return m_revision; }
    bool isUInputAvailable() const;
    QString uinputStatusMessage() const;

    bool isTransmitting() const;
    QString activeTransmittingKey() const;
    Q_INVOKABLE bool isTransmittingForTab(const QString &tabName) const;
    Q_INVOKABLE bool isTransmittingKey(const QString &key) const;
    Q_INVOKABLE void testTriggerKey(const QString &keyName, int durationMs = 2000);

    Q_INVOKABLE QVariantList profilesList() const;
    Q_INVOKABLE QVariantMap getProfile(int index) const;
    Q_INVOKABLE void setProfile(int index, const QVariantMap &data);
    Q_INVOKABLE int addProfile(const QString &name = QString());
    Q_INVOKABLE bool removeProfile(int index);

    Q_INVOKABLE QString activeBadgeForTab(const QString &tabName) const;
    Q_INVOKABLE bool hasActionForTab(const QString &tabName) const;
    Q_INVOKABLE QString keyNameFromEvent(int qtKey, quint32 nativeScanCode = 0) const;

    Actions::Profile profileForTab(const QString &tabName) const;

    // Playback Coordination
    void coordinatePlayback(SoundTileController *tile, const QString &tabName, std::function<void()> onReadyToPlay);
    void coordinatePlaybackFinished(SoundTileController *tile);
    void stopAll();

    // Tab synchronization
    void handleTabRenamed(const QString &oldName, const QString &newName);
    void handleTabRemoved(const QString &tabName);

    // Serialization
    QJsonArray toJson() const;
    void fromJson(const QJsonArray &array);

signals:
    void profilesChanged();
    void uinputStatusChanged();
    void transmissionChanged();
    void actionStarted(const QString &profileName, const QString &key, int mode);
    void actionEnded(const QString &profileName, const QString &key);

private:
    struct Session {
        QString keyName;
        QString tabName;
        Actions::TalkMode mode;
        int bufferMs;
        QSet<SoundTileController*> activeTiles;
        QTimer *leadInTimer = nullptr;
        QTimer *tailTimer = nullptr;
        bool isOpened = false;
        QList<std::function<void()>> pendingCallbacks;
    };

    QList<Actions::Profile> m_profiles;
    UInputChannel *m_uinput = nullptr;
    QMap<QString, Session*> m_sessions; // keyName -> Session
    QMap<SoundTileController*, QString> m_tileToKey;
    int m_revision = 0;
    QTimer *m_testTimer = nullptr;
    QString m_testKeyName;
    void notifyProfilesChanged();
    void cleanupSession(Session *s);
};

#endif // ACTIONSCONTROLLER_H
