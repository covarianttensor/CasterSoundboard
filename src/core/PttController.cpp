#include "PttController.h"
#include "UInputChannel.h"
#include "SoundTileController.h"

#include <QUuid>
#include <QDebug>

namespace {
QString formatKeyBadge(const QString &rawKey) {
    if (rawKey.startsWith("KEY_")) {
        QString sub = rawKey.mid(4);
        if (sub == "SPACE") return "Space";
        if (sub == "DOT") return ".";
        if (sub == "COMMA") return ",";
        if (sub == "LEFTCTRL") return "Ctrl";
        if (sub == "LEFTSHIFT") return "Shift";
        if (sub == "LEFTALT") return "Alt";
        return sub;
    }
    return rawKey;
}
}

PttController::PttController(QObject *parent)
    : QObject(parent)
    , m_uinput(new UInputChannel(this))
{
}

PttController::~PttController()
{
    stopAll();
}

void PttController::notifyProfilesChanged()
{
    ++m_revision;
    emit profilesChanged();
}

bool PttController::isUInputAvailable() const
{
    return m_uinput && m_uinput->isAvailable();
}

QString PttController::uinputStatusMessage() const
{
    return m_uinput ? m_uinput->statusMessage() : QString();
}

bool PttController::isTransmitting() const
{
    return !m_sessions.isEmpty() || !m_testKeyName.isEmpty();
}

QString PttController::activeTransmittingKey() const
{
    if (!m_testKeyName.isEmpty())
        return m_testKeyName;
    if (!m_sessions.isEmpty())
        return m_sessions.firstKey();
    return QString();
}

QStringList PttController::activeTransmittingKeys() const
{
    QStringList keys = m_sessions.keys();
    if (!m_testKeyName.isEmpty() && !keys.contains(m_testKeyName)) {
        keys.append(m_testKeyName);
    }
    return keys;
}

bool PttController::isTransmittingForTab(const QString &tabName) const
{
    if (tabName.isEmpty())
        return false;

    Ptt::Profile prof = profileForTab(tabName);
    if (prof.mode == Ptt::ModeNone || !prof.isValid())
        return false;

    const QString activeKey = prof.activeKey();

    if (!m_testKeyName.isEmpty() && m_testKeyName == activeKey)
        return true;

    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        if (it.value()->tabName == tabName)
            return true;
    }

    return m_sessions.contains(activeKey);
}

bool PttController::isTransmittingKey(const QString &key) const
{
    if (key.isEmpty())
        return false;
    return m_sessions.contains(key) || m_testKeyName == key;
}

void PttController::testTriggerKey(const QString &keyName, int durationMs)
{
    if (keyName.isEmpty() || !m_uinput || !m_uinput->isAvailable())
        return;

    if (m_testTimer) {
        m_testTimer->stop();
        m_testTimer->deleteLater();
        m_testTimer = nullptr;
        if (!m_testKeyName.isEmpty()) {
            m_uinput->sendKeyUp(m_testKeyName);
            m_testKeyName.clear();
        }
    }

    m_testKeyName = keyName;
    m_uinput->sendKeyDown(keyName);
    emit transmissionChanged();

    m_testTimer = new QTimer(this);
    m_testTimer->setSingleShot(true);
    connect(m_testTimer, &QTimer::timeout, this, [this]() {
        if (!m_testKeyName.isEmpty() && m_uinput) {
            m_uinput->sendKeyUp(m_testKeyName);
            m_testKeyName.clear();
        }
        if (m_testTimer) {
            m_testTimer->deleteLater();
            m_testTimer = nullptr;
        }
        emit transmissionChanged();
    });
    m_testTimer->start(qMax(100, durationMs));
}

QVariantList PttController::profilesList() const
{
    QVariantList list;
    for (int i = 0; i < m_profiles.size(); ++i) {
        list.append(getProfile(i));
    }
    return list;
}

QVariantMap PttController::getProfile(int index) const
{
    QVariantMap map;
    if (index < 0 || index >= m_profiles.size())
        return map;

    const auto &p = m_profiles.at(index);
    map["index"] = index;
    map["id"] = p.id;
    map["name"] = p.name;
    map["mode"] = static_cast<int>(p.mode);
    map["pinnedTabName"] = p.pinnedTabName;
    map["pttKey"] = p.pttKey;
    map["muteKey"] = p.muteKey;
    map["bufferMs"] = p.bufferMs;
    return map;
}

void PttController::setProfile(int index, const QVariantMap &data)
{
    if (index < 0 || index >= m_profiles.size())
        return;

    auto &p = m_profiles[index];
    if (data.contains("name")) p.name = data["name"].toString();
    if (data.contains("mode")) p.mode = static_cast<Ptt::TalkMode>(data["mode"].toInt());
    if (data.contains("pinnedTabName")) p.pinnedTabName = data["pinnedTabName"].toString();
    if (data.contains("pttKey")) p.pttKey = data["pttKey"].toString();
    if (data.contains("muteKey")) p.muteKey = data["muteKey"].toString();
    if (data.contains("bufferMs")) p.bufferMs = qBound(0, data["bufferMs"].toInt(), 5000);

    notifyProfilesChanged();
}

int PttController::addProfile(const QString &name)
{
    Ptt::Profile p;
    p.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    p.name = name.isEmpty() ? tr("New PTT Profile %1").arg(m_profiles.size() + 1) : name;
    p.mode = Ptt::ModePushToTalk;
    p.pttKey = "KEY_V";
    p.muteKey = "KEY_M";
    p.bufferMs = 150;

    m_profiles.append(p);
    notifyProfilesChanged();
    return m_profiles.size() - 1;
}

bool PttController::renameProfile(int index, const QString &newName)
{
    if (index < 0 || index >= m_profiles.size())
        return false;

    QString clean = newName.trimmed();
    if (clean.isEmpty())
        return false;

    if (m_profiles[index].name == clean)
        return true;

    m_profiles[index].name = clean;
    notifyProfilesChanged();
    return true;
}

bool PttController::removeProfile(int index)
{
    if (index < 0 || index >= m_profiles.size())
        return false;

    m_profiles.removeAt(index);
    notifyProfilesChanged();
    return true;
}

QString PttController::activeBadgeForTab(const QString &tabName) const
{
    if (tabName.isEmpty())
        return QString();

    for (const auto &p : m_profiles) {
        if (p.pinnedTabName == tabName && p.mode != Ptt::ModeNone) {
            if (p.mode == Ptt::ModePushToTalk) {
                return QString("PTT: %1").arg(formatKeyBadge(p.pttKey));
            } else if (p.mode == Ptt::ModeMuteToggle) {
                return QString("Mute: %1").arg(formatKeyBadge(p.muteKey));
            }
        }
    }
    return QString();
}

bool PttController::hasPttForTab(const QString &tabName) const
{
    return !activeBadgeForTab(tabName).isEmpty();
}

void PttController::setIsCapturingKey(bool capturing)
{
    if (m_isCapturingKey != capturing) {
        m_isCapturingKey = capturing;
        emit isCapturingKeyChanged(capturing);
    }
}

QString PttController::keyNameFromEvent(int qtKey, quint32 nativeScanCode, int convention) const
{
    auto conv = static_cast<UInputChannel::ScanCodeConvention>(convention);
    return UInputChannel::nameFromQtKey(qtKey, nativeScanCode, conv);
}

Ptt::Profile PttController::profileForTab(const QString &tabName) const
{
    if (tabName.isEmpty())
        return Ptt::Profile();

    for (const auto &p : m_profiles) {
        if (p.pinnedTabName == tabName)
            return p;
    }
    return Ptt::Profile();
}

void PttController::handleTabRenamed(const QString &oldName, const QString &newName)
{
    bool changed = false;
    for (auto &p : m_profiles) {
        if (p.pinnedTabName == oldName) {
            p.pinnedTabName = newName;
            changed = true;
        }
    }
    if (changed)
        notifyProfilesChanged();
}

void PttController::handleTabRemoved(const QString &tabName)
{
    bool changed = false;
    for (auto &p : m_profiles) {
        if (p.pinnedTabName == tabName) {
            p.pinnedTabName.clear();
            changed = true;
        }
    }
    if (changed)
        notifyProfilesChanged();
}

void PttController::handleTabCloned(const QString &sourceTabName, const QString &newTabName)
{
    if (sourceTabName.isEmpty() || newTabName.isEmpty())
        return;

    bool changed = false;
    QList<Ptt::Profile> newProfiles;
    QSet<QString> existingNames;
    for (const auto &p : m_profiles) {
        existingNames.insert(p.name);
    }

    for (const auto &p : m_profiles) {
        if (p.pinnedTabName == sourceTabName) {
            Ptt::Profile clone = p;
            clone.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            QString cloneName = QString("%1 (Copy)").arg(p.name);
            int copyNum = 2;
            while (existingNames.contains(cloneName)) {
                cloneName = QString("%1 (Copy %2)").arg(p.name).arg(copyNum++);
            }
            clone.name = cloneName;
            clone.pinnedTabName = newTabName;
            newProfiles.append(clone);
            existingNames.insert(cloneName);
            changed = true;
        }
    }

    if (changed) {
        m_profiles.append(newProfiles);
        notifyProfilesChanged();
    }
}

void PttController::coordinatePlayback(SoundTileController *tile, const QString &tabName, std::function<void()> onReadyToPlay)
{
    if (!tile) {
        if (onReadyToPlay) onReadyToPlay();
        return;
    }

    Ptt::Profile prof = profileForTab(tabName);
    if (prof.mode == Ptt::ModeNone || !prof.isValid()) {
        if (onReadyToPlay) onReadyToPlay();
        return;
    }

    const QString key = prof.activeKey();
    Session *session = m_sessions.value(key, nullptr);

    if (!session) {
        session = new Session();
        session->keyName = key;
        session->tabName = tabName;
        session->mode = prof.mode;
        session->bufferMs = prof.bufferMs;
        session->isOpened = false;
        m_sessions.insert(key, session);
    } else {
        session->tabName = tabName;
    }

    // Cancel any tail release timer
    if (session->tailTimer) {
        session->tailTimer->stop();
        session->tailTimer->deleteLater();
        session->tailTimer = nullptr;
    }

    session->activeTiles.insert(tile);
    m_tileToKey.insert(tile, key);

    if (session->isOpened) {
        // Session already active and audio gate is open; play immediately
        if (onReadyToPlay) onReadyToPlay();
        return;
    }

    // Session opening phase
    if (prof.mode == Ptt::ModePushToTalk) {
        m_uinput->sendKeyDown(key);
    } else if (prof.mode == Ptt::ModeMuteToggle) {
        m_uinput->sendKeyTap(key, 80);
    }
    emit pttStarted(prof.name, key, static_cast<int>(prof.mode));
    emit transmissionChanged();

    if (prof.bufferMs <= 0) {
        session->isOpened = true;
        if (onReadyToPlay) onReadyToPlay();
        return;
    }

    session->pendingCallbacks.append(onReadyToPlay);

    if (!session->leadInTimer) {
        session->leadInTimer = new QTimer(this);
        session->leadInTimer->setSingleShot(true);
        connect(session->leadInTimer, &QTimer::timeout, this, [this, session]() {
            session->isOpened = true;
            session->leadInTimer->deleteLater();
            session->leadInTimer = nullptr;

            auto cbs = session->pendingCallbacks;
            session->pendingCallbacks.clear();
            for (auto &cb : cbs) {
                if (cb) cb();
            }
        });
        session->leadInTimer->start(prof.bufferMs);
    }
}

void PttController::coordinatePlaybackFinished(SoundTileController *tile)
{
    if (!tile || !m_tileToKey.contains(tile))
        return;

    const QString key = m_tileToKey.take(tile);
    Session *session = m_sessions.value(key, nullptr);
    if (!session)
        return;

    session->activeTiles.remove(tile);

    // If other sounds are still playing on this key, keep session open
    if (!session->activeTiles.isEmpty())
        return;

    // Last sound ended: begin tail buffer timer before key release
    if (session->bufferMs <= 0) {
        cleanupSession(session);
        return;
    }

    if (!session->tailTimer) {
        session->tailTimer = new QTimer(this);
        session->tailTimer->setSingleShot(true);
        connect(session->tailTimer, &QTimer::timeout, this, [this, session]() {
            cleanupSession(session);
        });
        session->tailTimer->start(session->bufferMs);
    }
}

void PttController::cleanupSession(Session *s)
{
    if (!s)
        return;

    if (s->leadInTimer) {
        s->leadInTimer->stop();
        s->leadInTimer->deleteLater();
        s->leadInTimer = nullptr;
    }
    if (s->tailTimer) {
        s->tailTimer->stop();
        s->tailTimer->deleteLater();
        s->tailTimer = nullptr;
    }

    if (s->mode == Ptt::ModePushToTalk) {
        m_uinput->sendKeyUp(s->keyName);
    } else if (s->mode == Ptt::ModeMuteToggle) {
        m_uinput->sendKeyTap(s->keyName, 80);
    }

    emit pttEnded(s->keyName, s->keyName);

    m_sessions.remove(s->keyName);
    delete s;
    emit transmissionChanged();
}

void PttController::stopAll()
{
    if (m_testTimer) {
        m_testTimer->stop();
        m_testTimer->deleteLater();
        m_testTimer = nullptr;
    }
    if (!m_testKeyName.isEmpty()) {
        if (m_uinput) m_uinput->sendKeyUp(m_testKeyName);
        m_testKeyName.clear();
    }

    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        Session *s = it.value();
        if (s->leadInTimer) {
            s->leadInTimer->stop();
            s->leadInTimer->deleteLater();
        }
        if (s->tailTimer) {
            s->tailTimer->stop();
            s->tailTimer->deleteLater();
        }
        if (s->mode == Ptt::ModePushToTalk) {
            m_uinput->sendKeyUp(s->keyName);
        } else if (s->mode == Ptt::ModeMuteToggle) {
            m_uinput->sendKeyTap(s->keyName, 80);
        }
        delete s;
    }
    m_sessions.clear();
    m_tileToKey.clear();
    emit transmissionChanged();
}

QJsonArray PttController::toJson() const
{
    QJsonArray array;
    for (const auto &p : m_profiles) {
        array.append(p.toJson());
    }
    return array;
}

void PttController::fromJson(const QJsonArray &array)
{
    stopAll();
    m_profiles.clear();
    for (const auto &val : array) {
        if (val.isObject()) {
            m_profiles.append(Ptt::Profile::fromJson(val.toObject()));
        }
    }
    notifyProfilesChanged();
}
