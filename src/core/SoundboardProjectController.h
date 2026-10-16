#ifndef SOUNDBOARDPROJECTCONTROLLER_H
#define SOUNDBOARDPROJECTCONTROLLER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QAudioDevice>
#include <QMediaDevices>
#include "SoundboardTabModel.h"

class SoundboardGridModel;
class SoundTileController;

class SoundboardProjectController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(SoundboardTabModel* tabModel READ tabModel CONSTANT)
    Q_PROPERTY(QString filePath READ filePath NOTIFY filePathChanged)
    Q_PROPERTY(bool isDirty READ isDirty NOTIFY isDirtyChanged)
    Q_PROPERTY(int windowWidth READ windowWidth WRITE setWindowWidth NOTIFY windowGeometryChanged)
    Q_PROPERTY(int windowHeight READ windowHeight WRITE setWindowHeight NOTIFY windowGeometryChanged)
    Q_PROPERTY(bool isFullScreen READ isFullScreen WRITE setIsFullScreen NOTIFY isFullScreenChanged)
    Q_PROPERTY(qreal tileScale READ tileScale WRITE setTileScale NOTIFY tileScaleChanged)
    Q_PROPERTY(qreal masterVolume READ masterVolume WRITE setMasterVolume NOTIFY masterVolumeChanged)
    Q_PROPERTY(bool isDuckingActive READ isDuckingActive WRITE setIsDuckingActive NOTIFY isDuckingActiveChanged)
    Q_PROPERTY(qreal duckingVolumeRatio READ duckingVolumeRatio WRITE setDuckingVolumeRatio NOTIFY duckingVolumeRatioChanged)
    Q_PROPERTY(QString outputDeviceName READ outputDeviceName WRITE setOutputDeviceName NOTIFY outputDeviceNameChanged)
    Q_PROPERTY(QStringList availableAudioOutputs READ availableAudioOutputs NOTIFY availableAudioOutputsChanged)
    Q_PROPERTY(int hotkeyScope READ hotkeyScope WRITE setHotkeyScope NOTIFY hotkeyScopeChanged)

public:
    enum HotkeyScope {
        TabScoped = 0, // Mode B: Sound Banks (Tab-Scoped Tiles) - default
        Global = 1     // Mode A: Global across all tabs
    };
    Q_ENUM(HotkeyScope)

    explicit SoundboardProjectController(QObject *parent = nullptr);
    ~SoundboardProjectController() override;

    SoundboardTabModel* tabModel() const { return m_tabModel; }

    QString filePath() const { return m_filePath; }
    bool isDirty() const { return m_isDirty; }

    int windowWidth() const { return m_windowWidth; }
    void setWindowWidth(int w);

    int windowHeight() const { return m_windowHeight; }
    void setWindowHeight(int h);

    bool isFullScreen() const { return m_isFullScreen; }
    void setIsFullScreen(bool fs);

    qreal tileScale() const { return m_tileScale; }
    void setTileScale(qreal scale);

    qreal masterVolume() const { return m_masterVolume; }
    void setMasterVolume(qreal vol);

    bool isDuckingActive() const { return m_isDuckingActive; }
    void setIsDuckingActive(bool active);

    qreal duckingVolumeRatio() const { return m_duckingVolumeRatio; }
    void setDuckingVolumeRatio(qreal ratio);

    QString outputDeviceName() const { return m_outputDeviceName; }
    void setOutputDeviceName(const QString &deviceName);

    int hotkeyScope() const { return m_hotkeyScope; }
    void setHotkeyScope(int scope);

    QStringList availableAudioOutputs() const;
    Q_INVOKABLE void refreshAudioDevices();

    // Serialization
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &json);

public slots:
    bool newProject();
    bool loadProject(const QString &filePath);
    bool saveProject(const QString &filePath);
    bool save();

    void stopAllSounds();
    void toggleAudioDucking();

signals:
    void filePathChanged();
    void isDirtyChanged();
    void windowGeometryChanged();
    void isFullScreenChanged();
    void tileScaleChanged();
    void masterVolumeChanged();
    void isDuckingActiveChanged();
    void duckingVolumeRatioChanged();
    void outputDeviceNameChanged();
    void availableAudioOutputsChanged();
    void hotkeyScopeChanged();
    void projectLoaded();
    void projectSaved();
    void errorOccurred(const QString &errorMessage);

private:
    void setDirty(bool dirty);
    void applyDuckingToAllTiles();
    void applyOutputDeviceToAllTiles();

    SoundboardTabModel *m_tabModel{nullptr};
    QMediaDevices *m_mediaDevices{nullptr};
    QString m_filePath;
    bool m_isDirty{false};

    // Workspace settings
    int m_windowWidth{1024};
    int m_windowHeight{720};
    bool m_isFullScreen{false};
    qreal m_tileScale{1.0};
    qreal m_masterVolume{1.0};
    bool m_isDuckingActive{false};
    qreal m_duckingVolumeRatio{0.25};
    QString m_outputDeviceName;
    int m_hotkeyScope{TabScoped};
};

#endif // SOUNDBOARDPROJECTCONTROLLER_H
