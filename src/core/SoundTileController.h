#ifndef SOUNDTILECONTROLLER_H
#define SOUNDTILECONTROLLER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QKeySequence>
#include <QJsonObject>
#include <QMediaPlayer>
#include <QAudioOutput>

class SoundTileController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString hotkey READ hotkey WRITE setHotkey NOTIFY hotkeyChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool isLooped READ isLooped WRITE setIsLooped NOTIFY isLoopedChanged)
    Q_PROPERTY(TriggerStyle triggerStyle READ triggerStyle WRITE setTriggerStyle NOTIFY triggerStyleChanged)
    Q_PROPERTY(bool hasCueRegion READ hasCueRegion WRITE setHasCueRegion NOTIFY hasCueRegionChanged)
    Q_PROPERTY(qint64 cueBeginMs READ cueBeginMs WRITE setCueBeginMs NOTIFY cueBeginMsChanged)
    Q_PROPERTY(qint64 cueEndMs READ cueEndMs WRITE setCueEndMs NOTIFY cueEndMsChanged)
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool hasVideo READ hasVideo NOTIFY hasVideoChanged)
    Q_PROPERTY(QMediaPlayer::PlaybackState playbackState READ playbackState NOTIFY playbackStateChanged)
    Q_PROPERTY(qint64 positionMs READ positionMs NOTIFY positionMsChanged)
    Q_PROPERTY(qint64 durationMs READ durationMs NOTIFY durationMsChanged)
    Q_PROPERTY(QObject* player READ playerObject CONSTANT)
    Q_PROPERTY(QObject* videoOutput READ videoOutput WRITE setVideoOutput NOTIFY videoOutputChanged)

public:
    enum TriggerStyle {
        PlayPause = 0,
        PlayStop,
        PlayAgain,
        SoundEffect
    };
    Q_ENUM(TriggerStyle)

    explicit SoundTileController(QObject *parent = nullptr);
    explicit SoundTileController(const QString &id, QObject *parent = nullptr);
    ~SoundTileController() override;

    QString id() const { return m_id; }
    QString filePath() const { return m_filePath; }
    void setFilePath(const QString &path);

    QString displayName() const { return m_displayName; }
    void setDisplayName(const QString &name);

    QString hotkey() const { return m_hotkey.toString(QKeySequence::NativeText); }
    QKeySequence keySequence() const { return m_hotkey; }
    void setHotkey(const QString &hotkeyString);
    void setKeySequence(const QKeySequence &seq);

    qreal volume() const { return m_volume; }
    void setVolume(qreal vol);

    bool isLooped() const { return m_isLooped; }
    void setIsLooped(bool loop);

    TriggerStyle triggerStyle() const { return m_triggerStyle; }
    void setTriggerStyle(TriggerStyle style);

    bool hasCueRegion() const { return m_hasCueRegion; }
    void setHasCueRegion(bool enabled);

    qint64 cueBeginMs() const { return m_cueBeginMs; }
    void setCueBeginMs(qint64 ms);

    qint64 cueEndMs() const { return m_cueEndMs; }
    void setCueEndMs(qint64 ms);

    QString color() const { return m_color; }
    void setColor(const QString &c);

    bool hasVideo() const { return m_hasVideo; }
    QMediaPlayer::PlaybackState playbackState() const { return m_playbackState; }
    qint64 positionMs() const { return m_positionMs; }
    qint64 durationMs() const { return m_durationMs; }

    QObject* playerObject() const { return m_player; }
    QMediaPlayer* mediaPlayer() const { return m_player; }
    QAudioOutput* audioOutput() const { return m_audioOutput; }

    Q_INVOKABLE void setVideoOutput(QObject *output);
    Q_INVOKABLE QObject* videoOutput() const;

    // Serialization
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &json);

public slots:
    void trigger();
    void play();
    void pause();
    void stop();
    void seek(qint64 ms);
    void setAudioOutputDevice(const QAudioDevice &device);
    void setDuckingAttenuation(qreal factor);
    void onPositionChanged(qint64 pos);
    void onDurationChanged(qint64 dur);

signals:
    void filePathChanged();
    void displayNameChanged();
    void hotkeyChanged();
    void volumeChanged();
    void isLoopedChanged();
    void triggerStyleChanged();
    void hasCueRegionChanged();
    void cueBeginMsChanged();
    void cueEndMsChanged();
    void colorChanged();
    void hasVideoChanged();
    void playbackStateChanged();
    void positionMsChanged();
    void durationMsChanged();
    void videoOutputChanged();
    void playbackFinished();

private slots:
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onTracksChanged();

private:
    void updateEffectiveVolume();
    void autoDetectDisplayName(const QString &path);

    QString m_id;
    QString m_filePath;
    QString m_displayName;
    QKeySequence m_hotkey;
    qreal m_volume{1.0};
    qreal m_duckingFactor{1.0};
    bool m_isLooped{false};
    TriggerStyle m_triggerStyle{PlayPause};
    bool m_hasCueRegion{false};
    qint64 m_cueBeginMs{0};
    qint64 m_cueEndMs{0};
    QString m_color{"#3A3D40"};
    bool m_hasVideo{false};
    QMediaPlayer::PlaybackState m_playbackState{QMediaPlayer::StoppedState};
    qint64 m_positionMs{0};
    qint64 m_durationMs{0};

    QMediaPlayer *m_player{nullptr};
    QAudioOutput *m_audioOutput{nullptr};
};

#endif // SOUNDTILECONTROLLER_H
