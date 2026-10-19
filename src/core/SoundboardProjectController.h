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
class PttController;

class SoundboardProjectController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(SoundboardTabModel* tabModel READ tabModel CONSTANT)
    Q_PROPERTY(QString filePath READ filePath NOTIFY filePathChanged)
    Q_PROPERTY(bool isDirty READ isDirty NOTIFY isDirtyChanged)
    Q_PROPERTY(int windowWidth READ windowWidth WRITE setWindowWidth NOTIFY windowGeometryChanged)
    Q_PROPERTY(int windowHeight READ windowHeight WRITE setWindowHeight NOTIFY windowGeometryChanged)
    Q_PROPERTY(bool isFullScreen READ isFullScreen WRITE setIsFullScreen NOTIFY isFullScreenChanged)
    Q_PROPERTY(bool isMaximized READ isMaximized WRITE setIsMaximized NOTIFY isMaximizedChanged)
    Q_PROPERTY(qreal tileScale READ tileScale WRITE setTileScale NOTIFY tileScaleChanged)
    Q_PROPERTY(qreal masterVolume READ masterVolume WRITE setMasterVolume NOTIFY masterVolumeChanged)
    Q_PROPERTY(bool isDuckingActive READ isDuckingActive WRITE setIsDuckingActive NOTIFY isDuckingActiveChanged)
    Q_PROPERTY(qreal duckingVolumeRatio READ duckingVolumeRatio WRITE setDuckingVolumeRatio NOTIFY duckingVolumeRatioChanged)
    Q_PROPERTY(QString outputDeviceName READ outputDeviceName WRITE setOutputDeviceName NOTIFY outputDeviceNameChanged)
    Q_PROPERTY(QStringList availableAudioOutputs READ availableAudioOutputs NOTIFY availableAudioOutputsChanged)
    Q_PROPERTY(int hotkeyScope READ hotkeyScope WRITE setHotkeyScope NOTIFY hotkeyScopeChanged)
    Q_PROPERTY(QString stopAllHotkey READ stopAllHotkey WRITE setStopAllHotkey NOTIFY stopAllHotkeyChanged)
    Q_PROPERTY(QString duckingHotkey READ duckingHotkey WRITE setDuckingHotkey NOTIFY duckingHotkeyChanged)
    Q_PROPERTY(QString openProjectHotkey READ openProjectHotkey WRITE setOpenProjectHotkey NOTIFY openProjectHotkeyChanged)
    Q_PROPERTY(QString saveProjectHotkey READ saveProjectHotkey WRITE setSaveProjectHotkey NOTIFY saveProjectHotkeyChanged)
    Q_PROPERTY(QString saveAsProjectHotkey READ saveAsProjectHotkey WRITE setSaveAsProjectHotkey NOTIFY saveAsProjectHotkeyChanged)
    Q_PROPERTY(QString newTabHotkey READ newTabHotkey WRITE setNewTabHotkey NOTIFY newTabHotkeyChanged)
    Q_PROPERTY(QString importDirectoryHotkey READ importDirectoryHotkey WRITE setImportDirectoryHotkey NOTIFY importDirectoryHotkeyChanged)
    Q_PROPERTY(QString renameTabHotkey READ renameTabHotkey WRITE setRenameTabHotkey NOTIFY renameTabHotkeyChanged)
    Q_PROPERTY(QString pttDialogHotkey READ pttDialogHotkey WRITE setPttDialogHotkey NOTIFY pttDialogHotkeyChanged)
    Q_PROPERTY(QString muteHotkeysHotkey READ muteHotkeysHotkey WRITE setMuteHotkeysHotkey NOTIFY muteHotkeysHotkeyChanged)
    Q_PROPERTY(PttController* pttController READ pttController CONSTANT)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
    Q_PROPERTY(bool isDevBuild READ isDevBuild CONSTANT)

public:
    enum HotkeyScope {
        TabScoped = 0, // Mode B: Sound Banks (Tab-Scoped Tiles) - default
        Global = 1     // Mode A: Global across all tabs
    };
    Q_ENUM(HotkeyScope)

    explicit SoundboardProjectController(QObject *parent = nullptr);
    ~SoundboardProjectController() override;

    SoundboardTabModel* tabModel() const { return m_tabModel; }
    PttController* pttController() const { return m_pttController; }

    QString appVersion() const;
    bool isDevBuild() const;

    QString filePath() const { return m_filePath; }
    bool isDirty() const { return m_isDirty; }

    int windowWidth() const { return m_windowWidth; }
    void setWindowWidth(int w);

    int windowHeight() const { return m_windowHeight; }
    void setWindowHeight(int h);

    bool isFullScreen() const { return m_isFullScreen; }
    void setIsFullScreen(bool fs);

    bool isMaximized() const { return m_isMaximized; }
    void setIsMaximized(bool max);

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

    QString stopAllHotkey() const { return m_stopAllHotkey; }
    void setStopAllHotkey(const QString &hotkey);

    QString duckingHotkey() const { return m_duckingHotkey; }
    void setDuckingHotkey(const QString &hotkey);

    QString openProjectHotkey() const { return m_openProjectHotkey; }
    void setOpenProjectHotkey(const QString &hotkey);

    QString saveProjectHotkey() const { return m_saveProjectHotkey; }
    void setSaveProjectHotkey(const QString &hotkey);

    QString saveAsProjectHotkey() const { return m_saveAsProjectHotkey; }
    void setSaveAsProjectHotkey(const QString &hotkey);

    QString newTabHotkey() const { return m_newTabHotkey; }
    void setNewTabHotkey(const QString &hotkey);

    QString importDirectoryHotkey() const { return m_importDirectoryHotkey; }
    void setImportDirectoryHotkey(const QString &hotkey);

    QString renameTabHotkey() const { return m_renameTabHotkey; }
    void setRenameTabHotkey(const QString &hotkey);

    QString pttDialogHotkey() const { return m_pttDialogHotkey; }
    void setPttDialogHotkey(const QString &hotkey);

    QString muteHotkeysHotkey() const { return m_muteHotkeysHotkey; }
    void setMuteHotkeysHotkey(const QString &hotkey);

    QStringList availableAudioOutputs() const;
    QAudioDevice currentAudioOutputDevice() const;
    Q_INVOKABLE void refreshAudioDevices();
    Q_INVOKABLE void copyToClipboard(const QString &text);
    Q_INVOKABLE bool isLegacyFile(const QString &filePath) const;

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
    void isMaximizedChanged();
    void tileScaleChanged();
    void masterVolumeChanged();
    void isDuckingActiveChanged();
    void duckingVolumeRatioChanged();
    void outputDeviceNameChanged();
    void availableAudioOutputsChanged();
    void hotkeyScopeChanged();
    void stopAllHotkeyChanged();
    void duckingHotkeyChanged();
    void openProjectHotkeyChanged();
    void saveProjectHotkeyChanged();
    void saveAsProjectHotkeyChanged();
    void newTabHotkeyChanged();
    void importDirectoryHotkeyChanged();
    void renameTabHotkeyChanged();
    void pttDialogHotkeyChanged();
    void muteHotkeysHotkeyChanged();
    void projectLoaded();
    void projectSaved();
    void errorOccurred(const QString &errorMessage);

private:
    void setDirty(bool dirty);
    void connectGridModel(SoundboardGridModel *grid);
    void applyDuckingToAllTiles();
    void applyOutputDeviceToAllTiles();
    void setupPttCoordination();

    SoundboardTabModel *m_tabModel{nullptr};
    PttController *m_pttController{nullptr};
    QMediaDevices *m_mediaDevices{nullptr};
    QString m_filePath;
    bool m_isDirty{false};

    // Workspace settings
    int m_windowWidth{1180};
    int m_windowHeight{720};
    bool m_isFullScreen{false};
    bool m_isMaximized{false};
    qreal m_tileScale{1.0};
    qreal m_masterVolume{1.0};
    bool m_isDuckingActive{false};
    qreal m_duckingVolumeRatio{0.25};
    QString m_outputDeviceName;
    int m_hotkeyScope{TabScoped};
    QString m_stopAllHotkey{"Space"};
    QString m_duckingHotkey{"Ctrl+D"};
    QString m_openProjectHotkey{"Ctrl+O"};
    QString m_saveProjectHotkey{"Ctrl+S"};
    QString m_saveAsProjectHotkey{"Ctrl+Shift+S"};
    QString m_newTabHotkey{"Ctrl+N"};
    QString m_importDirectoryHotkey{"Ctrl+I"};
    QString m_renameTabHotkey{"F2"};
    QString m_pttDialogHotkey{"Ctrl+Shift+P"};
    QString m_muteHotkeysHotkey{"Ctrl+M"};
};

#endif // SOUNDBOARDPROJECTCONTROLLER_H
