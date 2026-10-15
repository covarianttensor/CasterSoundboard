#include "SoundTileController.h"
#include <QFileInfo>
#include <QUuid>
#include <QMediaMetaData>
#include <QDebug>

SoundTileController::SoundTileController(QObject *parent)
    : SoundTileController(QUuid::createUuid().toString(QUuid::WithoutBraces), parent)
{
}

SoundTileController::SoundTileController(const QString &id, QObject *parent)
    : QObject(parent)
    , m_id(id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : id)
{
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);

    m_audioOutput->setVolume(static_cast<float>(m_volume));

    connect(m_player, &QMediaPlayer::positionChanged, this, &SoundTileController::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &SoundTileController::onDurationChanged);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &SoundTileController::onPlaybackStateChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &SoundTileController::onMediaStatusChanged);
    connect(m_player, &QMediaPlayer::tracksChanged, this, &SoundTileController::onTracksChanged);
}

SoundTileController::~SoundTileController()
{
    if (m_player) {
        m_player->stop();
    }
}

void SoundTileController::setFilePath(const QString &path)
{
    if (m_filePath == path)
        return;

    m_filePath = path;
    if (m_displayName.isEmpty() || m_displayName == "<Drop File>") {
        autoDetectDisplayName(path);
    }

    if (!m_filePath.isEmpty()) {
        QUrl url = QUrl::fromUserInput(m_filePath);
        if (url.isLocalFile() || !url.isValid()) {
            m_player->setSource(QUrl::fromLocalFile(m_filePath));
        } else {
            m_player->setSource(url);
        }
    } else {
        m_player->setSource(QUrl());
    }

    emit filePathChanged();
}

void SoundTileController::autoDetectDisplayName(const QString &path)
{
    if (path.isEmpty()) {
        setDisplayName("<Drop File>");
        return;
    }
    QFileInfo info(path);
    setDisplayName(info.completeBaseName());
}

void SoundTileController::setDisplayName(const QString &name)
{
    if (m_displayName == name)
        return;

    m_displayName = name;
    emit displayNameChanged();
}

void SoundTileController::setHotkey(const QString &hotkeyString)
{
    setKeySequence(QKeySequence::fromString(hotkeyString, QKeySequence::NativeText));
}

void SoundTileController::setKeySequence(const QKeySequence &seq)
{
    if (m_hotkey == seq)
        return;

    m_hotkey = seq;
    emit hotkeyChanged();
}

void SoundTileController::setVolume(qreal vol)
{
    vol = qBound(0.0, vol, 1.0);
    if (qFuzzyCompare(m_volume, vol))
        return;

    m_volume = vol;
    updateEffectiveVolume();
    emit volumeChanged();
}

void SoundTileController::setDuckingAttenuation(qreal factor)
{
    factor = qBound(0.0, factor, 1.0);
    if (qFuzzyCompare(m_duckingFactor, factor))
        return;

    m_duckingFactor = factor;
    updateEffectiveVolume();
}

void SoundTileController::updateEffectiveVolume()
{
    if (m_audioOutput) {
        m_audioOutput->setVolume(static_cast<float>(m_volume * m_duckingFactor));
    }
}

void SoundTileController::setIsLooped(bool loop)
{
    if (m_isLooped == loop)
        return;

    m_isLooped = loop;
    emit isLoopedChanged();
}

void SoundTileController::setTriggerStyle(TriggerStyle style)
{
    if (m_triggerStyle == style)
        return;

    m_triggerStyle = style;
    emit triggerStyleChanged();
}

void SoundTileController::setHasCueRegion(bool enabled)
{
    if (m_hasCueRegion == enabled)
        return;

    m_hasCueRegion = enabled;
    emit hasCueRegionChanged();
}

void SoundTileController::setCueBeginMs(qint64 ms)
{
    ms = qMax(qint64(0), ms);
    if (m_cueBeginMs == ms)
        return;

    m_cueBeginMs = ms;
    emit cueBeginMsChanged();
}

void SoundTileController::setCueEndMs(qint64 ms)
{
    ms = qMax(qint64(0), ms);
    if (m_cueEndMs == ms)
        return;

    m_cueEndMs = ms;
    emit cueEndMsChanged();
}

void SoundTileController::setColor(const QString &c)
{
    if (m_color == c)
        return;

    m_color = c;
    emit colorChanged();
}

void SoundTileController::setAudioOutputDevice(const QAudioDevice &device)
{
    if (m_audioOutput) {
        m_audioOutput->setDevice(device);
    }
}

void SoundTileController::trigger()
{
    if (m_filePath.isEmpty())
        return;

    switch (m_triggerStyle) {
    case PlayPause:
        if (m_playbackState == QMediaPlayer::PlayingState) {
            pause();
        } else {
            play();
        }
        break;

    case PlayStop:
        if (m_playbackState == QMediaPlayer::PlayingState) {
            stop();
        } else {
            play();
        }
        break;

    case PlayAgain:
    case SoundEffect:
        seek(m_hasCueRegion ? m_cueBeginMs : 0);
        play();
        break;
    }
}

void SoundTileController::play()
{
    if (m_filePath.isEmpty())
        return;

    if (m_hasCueRegion) {
        qint64 startPos = m_cueBeginMs;
        if (m_player->position() < startPos || (m_cueEndMs > startPos && m_player->position() >= m_cueEndMs)) {
            m_player->setPosition(startPos);
        }
    }

    m_player->play();
}

void SoundTileController::pause()
{
    m_player->pause();
}

void SoundTileController::stop()
{
    m_player->stop();
    if (m_hasCueRegion) {
        m_player->setPosition(m_cueBeginMs);
    } else {
        m_player->setPosition(0);
    }
}

void SoundTileController::seek(qint64 ms)
{
    m_player->setPosition(ms);
}

void SoundTileController::onPositionChanged(qint64 pos)
{
    m_positionMs = pos;
    emit positionMsChanged();

    // Check cue boundary
    if (m_hasCueRegion && m_cueEndMs > m_cueBeginMs && pos >= m_cueEndMs) {
        if (m_isLooped) {
            m_player->setPosition(m_cueBeginMs);
        } else {
            stop();
            emit playbackFinished();
        }
    }
}

void SoundTileController::onDurationChanged(qint64 dur)
{
    m_durationMs = dur;
    emit durationMsChanged();

    if (m_hasCueRegion && m_cueEndMs == 0 && dur > 0) {
        setCueEndMs(dur);
    }
}

void SoundTileController::onPlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    if (m_playbackState == state)
        return;

    m_playbackState = state;
    emit playbackStateChanged();
}

void SoundTileController::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia) {
        if (m_isLooped) {
            seek(m_hasCueRegion ? m_cueBeginMs : 0);
            m_player->play();
        } else {
            m_playbackState = QMediaPlayer::StoppedState;
            emit playbackStateChanged();
            emit playbackFinished();
        }
    }
}

void SoundTileController::onTracksChanged()
{
    bool videoDetected = m_player->videoTracks().size() > 0;
    if (m_hasVideo != videoDetected) {
        m_hasVideo = videoDetected;
        emit hasVideoChanged();
    }
}

QJsonObject SoundTileController::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["filePath"] = m_filePath;
    json["displayName"] = m_displayName;
    json["hotkey"] = hotkey();
    json["volume"] = m_volume;
    json["isLooped"] = m_isLooped;
    json["triggerStyle"] = static_cast<int>(m_triggerStyle);
    json["color"] = m_color;

    QJsonObject cueObj;
    cueObj["enabled"] = m_hasCueRegion;
    cueObj["beginMs"] = m_cueBeginMs;
    cueObj["endMs"] = m_cueEndMs;
    json["cue"] = cueObj;

    return json;
}

bool SoundTileController::fromJson(const QJsonObject &json)
{
    if (json.contains("id")) {
        m_id = json["id"].toString();
    }
    if (json.contains("filePath")) {
        setFilePath(json["filePath"].toString());
    }
    if (json.contains("displayName")) {
        setDisplayName(json["displayName"].toString());
    }
    if (json.contains("hotkey")) {
        setHotkey(json["hotkey"].toString());
    }
    if (json.contains("volume")) {
        setVolume(json["volume"].toDouble(1.0));
    }
    if (json.contains("isLooped")) {
        setIsLooped(json["isLooped"].toBool(false));
    }
    if (json.contains("triggerStyle")) {
        setTriggerStyle(static_cast<TriggerStyle>(json["triggerStyle"].toInt(0)));
    }
    if (json.contains("color")) {
        setColor(json["color"].toString("#3A3D40"));
    }
    if (json.contains("cue") && json["cue"].isObject()) {
        QJsonObject cueObj = json["cue"].toObject();
        setHasCueRegion(cueObj["enabled"].toBool(false));
        setCueBeginMs(cueObj["beginMs"].toInteger(0));
        setCueEndMs(cueObj["endMs"].toInteger(0));
    }

    return true;
}

void SoundTileController::setVideoOutput(QObject *output)
{
    if (m_player) {
        m_player->setVideoOutput(output);
        emit videoOutputChanged();
    }
}

QObject* SoundTileController::videoOutput() const
{
    return m_player ? m_player->videoOutput() : nullptr;
}
