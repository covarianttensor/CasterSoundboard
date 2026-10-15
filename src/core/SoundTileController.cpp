#include "SoundTileController.h"
#include <QFileInfo>
#include <QUuid>
#include <QMediaMetaData>
#include <QFile>
#include <QProcess>
#include <QVector>
#include <QtMath>
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

    updateWaveform();

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

void SoundTileController::updateWaveform()
{
    if (m_filePath.isEmpty()) {
        if (!m_waveform.isEmpty()) {
            m_waveform.clear();
            emit waveformChanged();
        }
        return;
    }

    QString localPath = m_filePath;
    if (localPath.startsWith("file://")) {
        localPath = QUrl(localPath).toLocalFile();
    } else {
        QUrl url = QUrl::fromUserInput(localPath);
        if (url.isLocalFile()) {
            localPath = url.toLocalFile();
        }
    }

    QVariantList points;
    constexpr int numBins = 64;

    // First attempt direct WAV parsing if it's a RIFF PCM file
    QFile file(localPath);
    bool parsedDirect = false;
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray header = file.read(12);
        if (header.size() == 12 && header.startsWith("RIFF") && header.mid(8, 4) == "WAVE") {
            quint16 audioFormat = 0;
            quint16 numChannels = 0;
            quint16 blockAlign = 0;
            quint16 bitsPerSample = 0;
            qint64 dataOffset = 0;
            qint64 dataSize = 0;

            while (!file.atEnd()) {
                QByteArray chunkHeader = file.read(8);
                if (chunkHeader.size() < 8) break;
                QByteArray chunkId = chunkHeader.left(4);
                quint32 chunkSize = *reinterpret_cast<const quint32*>(chunkHeader.constData() + 4);

                if (chunkId == "fmt ") {
                    QByteArray fmtData = file.read(chunkSize);
                    if (fmtData.size() >= 16) {
                        audioFormat = *reinterpret_cast<const quint16*>(fmtData.constData());
                        numChannels = *reinterpret_cast<const quint16*>(fmtData.constData() + 2);
                        blockAlign = *reinterpret_cast<const quint16*>(fmtData.constData() + 12);
                        bitsPerSample = *reinterpret_cast<const quint16*>(fmtData.constData() + 14);
                    }
                } else if (chunkId == "data") {
                    dataOffset = file.pos();
                    dataSize = chunkSize;
                    break;
                } else {
                    file.seek(file.pos() + chunkSize);
                }
            }

            if (audioFormat == 1 && bitsPerSample == 16 && blockAlign > 0 && dataSize > 0) {
                file.seek(dataOffset);
                QByteArray audioBytes = file.read(dataSize);
                int totalSamples = audioBytes.size() / blockAlign;
                if (totalSamples > 0) {
                    const qint16 *samples = reinterpret_cast<const qint16*>(audioBytes.constData());
                    int channelStride = blockAlign / 2;
                    QVector<double> rawPeaks(numBins, 0.0);
                    double maxPeak = 0.0;

                    for (int b = 0; b < numBins; ++b) {
                        int startSample = static_cast<int>((static_cast<qint64>(b) * totalSamples) / numBins);
                        int endSample = static_cast<int>((static_cast<qint64>(b + 1) * totalSamples) / numBins);
                        if (endSample <= startSample) endSample = startSample + 1;
                        if (endSample > totalSamples) endSample = totalSamples;

                        double peak = 0.0;
                        int step = qMax(1, (endSample - startSample) / 256);
                        for (int s = startSample; s < endSample; s += step) {
                            qint16 val = samples[s * channelStride];
                            double absVal = qAbs(static_cast<double>(val));
                            if (absVal > peak) peak = absVal;
                        }
                        rawPeaks[b] = peak;
                        if (peak > maxPeak) maxPeak = peak;
                    }

                    for (int b = 0; b < numBins; ++b) {
                        double norm = (maxPeak > 0.0) ? (rawPeaks[b] / maxPeak) : 0.08;
                        norm = qMax(0.08, qMin(1.0, norm));
                        points.append(norm);
                    }
                    parsedDirect = true;
                }
            }
        }
        file.close();
    }

    // Fallback: If not PCM WAV (e.g. MP3, OGG, FLAC, M4A, video files with audio tracks, etc.), decode downsampled PCM via ffmpeg
    if (!parsedDirect) {
        QProcess proc;
        proc.start("ffmpeg", QStringList() << "-v" << "error" << "-threads" << "4" << "-i" << localPath
                                           << "-vn" << "-ac" << "1" << "-ar" << "2000"
                                           << "-f" << "s16le" << "-");
        if (proc.waitForFinished(5000)) {
            QByteArray pcmData = proc.readAllStandardOutput();
            int totalSamples = pcmData.size() / 2;
            if (totalSamples > 0) {
                const qint16 *samples = reinterpret_cast<const qint16*>(pcmData.constData());
                QVector<double> rawPeaks(numBins, 0.0);
                double maxPeak = 0.0;

                for (int b = 0; b < numBins; ++b) {
                    int startSample = static_cast<int>((static_cast<qint64>(b) * totalSamples) / numBins);
                    int endSample = static_cast<int>((static_cast<qint64>(b + 1) * totalSamples) / numBins);
                    if (endSample <= startSample) endSample = startSample + 1;
                    if (endSample > totalSamples) endSample = totalSamples;

                    double peak = 0.0;
                    int step = qMax(1, (endSample - startSample) / 128);
                    for (int s = startSample; s < endSample; s += step) {
                        double absVal = qAbs(static_cast<double>(samples[s]));
                        if (absVal > peak) peak = absVal;
                    }
                    rawPeaks[b] = peak;
                    if (peak > maxPeak) maxPeak = peak;
                }

                for (int b = 0; b < numBins; ++b) {
                    double norm = (maxPeak > 0.0) ? (rawPeaks[b] / maxPeak) : 0.08;
                    norm = qMax(0.08, qMin(1.0, norm));
                    points.append(norm);
                }
            }
        }
    }

    if (m_waveform != points) {
        m_waveform = points;
        emit waveformChanged();
    }
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
    if (m_durationMs > 0) {
        ms = qBound(qint64(0), ms, m_durationMs);
    } else {
        ms = qMax(qint64(0), ms);
    }
    m_player->setPosition(ms);
    m_positionMs = ms;
    emit positionMsChanged();
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
    } else if (json.contains("hasCueRegion")) {
        setHasCueRegion(json["hasCueRegion"].toBool(false));
        setCueBeginMs(json["cueBeginMs"].toInteger(0));
        setCueEndMs(json["cueEndMs"].toInteger(0));
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
