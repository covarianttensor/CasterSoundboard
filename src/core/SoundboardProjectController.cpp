#include "SoundboardProjectController.h"
#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"
#include "SoundTileController.h"
#include "ActionsController.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QGuiApplication>
#include <QClipboard>
#include <QDebug>

SoundboardProjectController::SoundboardProjectController(QObject *parent)
    : QObject(parent)
{
    m_tabModel = new SoundboardTabModel(this);
    m_actionsController = new ActionsController(this);
    m_mediaDevices = new QMediaDevices(this);

    connect(m_actionsController, &ActionsController::profilesChanged, this, [this]() {
        setupActionsCoordination();
        setDirty(true);
    });

    connect(m_mediaDevices, &QMediaDevices::audioOutputsChanged, this, [this]() {
        applyOutputDeviceToAllTiles();
        emit availableAudioOutputsChanged();
    });
    connect(m_tabModel, &QAbstractItemModel::dataChanged, this, [this]() {
        setupActionsCoordination();
        setDirty(true);
    });
    connect(m_tabModel, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex &, int first, int last) {
        for (int i = first; i <= last; ++i) {
            SoundboardGridModel *g = m_tabModel->gridModelAt(i);
            if (g) {
                connect(g, &QAbstractItemModel::rowsInserted, this, [this]() {
                    applyOutputDeviceToAllTiles();
                    setupActionsCoordination();
                });
            }
        }
        applyOutputDeviceToAllTiles();
        setupActionsCoordination();
        setDirty(true);
    });
    connect(m_tabModel, &QAbstractItemModel::rowsRemoved, this, [this]() {
        setupActionsCoordination();
        setDirty(true);
    });
    connect(m_tabModel, &QAbstractItemModel::rowsMoved, this, [this]() {
        setupActionsCoordination();
        setDirty(true);
    });
    connect(m_tabModel, &SoundboardTabModel::currentGridModelChanged, this, [this]() {
        applyOutputDeviceToAllTiles();
        setupActionsCoordination();
    });
    newProject();
}

SoundboardProjectController::~SoundboardProjectController()
{
}

void SoundboardProjectController::setWindowWidth(int w)
{
    w = qMax(400, w);
    if (m_windowWidth == w)
        return;
    m_windowWidth = w;
    setDirty(true);
    emit windowGeometryChanged();
}

void SoundboardProjectController::setWindowHeight(int h)
{
    h = qMax(300, h);
    if (m_windowHeight == h)
        return;
    m_windowHeight = h;
    setDirty(true);
    emit windowGeometryChanged();
}

void SoundboardProjectController::setIsFullScreen(bool fs)
{
    if (m_isFullScreen == fs)
        return;
    m_isFullScreen = fs;
    setDirty(true);
    emit isFullScreenChanged();
}

void SoundboardProjectController::setIsMaximized(bool max)
{
    if (m_isMaximized == max)
        return;
    m_isMaximized = max;
    setDirty(true);
    emit isMaximizedChanged();
}

void SoundboardProjectController::setTileScale(qreal scale)
{
    scale = qBound(0.5, scale, 2.5);
    if (qFuzzyCompare(m_tileScale, scale))
        return;
    m_tileScale = scale;
    setDirty(true);
    emit tileScaleChanged();
}

void SoundboardProjectController::setMasterVolume(qreal vol)
{
    vol = qBound(0.0, vol, 1.0);
    if (qFuzzyCompare(m_masterVolume, vol))
        return;
    m_masterVolume = vol;
    setDirty(true);
    emit masterVolumeChanged();
}

void SoundboardProjectController::setIsDuckingActive(bool active)
{
    if (m_isDuckingActive == active)
        return;
    m_isDuckingActive = active;
    applyDuckingToAllTiles();
    emit isDuckingActiveChanged();
}

void SoundboardProjectController::setDuckingVolumeRatio(qreal ratio)
{
    ratio = qBound(0.0, ratio, 1.0);
    if (qFuzzyCompare(m_duckingVolumeRatio, ratio))
        return;
    m_duckingVolumeRatio = ratio;
    if (m_isDuckingActive) {
        applyDuckingToAllTiles();
    }
    setDirty(true);
    emit duckingVolumeRatioChanged();
}

void SoundboardProjectController::setOutputDeviceName(const QString &deviceName)
{
    if (m_outputDeviceName == deviceName)
        return;
    m_outputDeviceName = deviceName;
    applyOutputDeviceToAllTiles();
    setDirty(true);
    emit outputDeviceNameChanged();
}

QStringList SoundboardProjectController::availableAudioOutputs() const
{
    QStringList list;
    list.append(tr("Default System Output"));
    const auto outputs = QMediaDevices::audioOutputs();
    for (const auto &dev : outputs) {
        QString desc = dev.description();
        if (!desc.isEmpty() && !list.contains(desc)) {
            list.append(desc);
        }
    }
    return list;
}

void SoundboardProjectController::refreshAudioDevices()
{
    emit availableAudioOutputsChanged();
}

void SoundboardProjectController::setHotkeyScope(int scope)
{
    scope = qBound(0, scope, 1);
    if (m_hotkeyScope == scope)
        return;
    m_hotkeyScope = scope;
    setDirty(true);
    emit hotkeyScopeChanged();
}

void SoundboardProjectController::setStopAllHotkey(const QString &hotkey)
{
    if (m_stopAllHotkey == hotkey)
        return;
    m_stopAllHotkey = hotkey;
    setDirty(true);
    emit stopAllHotkeyChanged();
}

void SoundboardProjectController::setDuckingHotkey(const QString &hotkey)
{
    if (m_duckingHotkey == hotkey)
        return;
    m_duckingHotkey = hotkey;
    setDirty(true);
    emit duckingHotkeyChanged();
}

void SoundboardProjectController::setDirty(bool dirty)
{
    if (m_isDirty == dirty)
        return;
    m_isDirty = dirty;
    emit isDirtyChanged();
}

bool SoundboardProjectController::newProject()
{
    stopAllSounds();

    m_tabModel->clear();
    m_tabModel->addTab("Sound Set 1");

    m_filePath.clear();
    m_isDirty = false;
    m_isDuckingActive = false;
    m_hotkeyScope = TabScoped;
    m_isMaximized = false;
    m_stopAllHotkey = "Space";
    m_duckingHotkey = "Ctrl+D";

    emit filePathChanged();
    emit isDirtyChanged();
    emit isDuckingActiveChanged();
    emit hotkeyScopeChanged();
    emit isMaximizedChanged();
    emit stopAllHotkeyChanged();
    emit duckingHotkeyChanged();
    return true;
}

bool SoundboardProjectController::loadProject(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred(tr("Could not open file for reading: %1").arg(file.errorString()));
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        emit errorOccurred(tr("Invalid JSON project file: %1").arg(parseError.errorString()));
        return false;
    }

    stopAllSounds();

    if (!fromJson(doc.object())) {
        emit errorOccurred(tr("Failed to parse project contents."));
        return false;
    }

    m_filePath = QFileInfo(filePath).absoluteFilePath();
    m_isDirty = false;

    emit filePathChanged();
    emit isDirtyChanged();
    emit projectLoaded();
    return true;
}

bool SoundboardProjectController::saveProject(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        emit errorOccurred(tr("Could not open file for writing: %1").arg(file.errorString()));
        return false;
    }

    QJsonObject root = toJson();
    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    m_filePath = QFileInfo(filePath).absoluteFilePath();
    m_isDirty = false;

    emit filePathChanged();
    emit isDirtyChanged();
    emit projectSaved();
    return true;
}

bool SoundboardProjectController::save()
{
    if (m_filePath.isEmpty())
        return false;
    return saveProject(m_filePath);
}

void SoundboardProjectController::stopAllSounds()
{
    if (m_actionsController) {
        m_actionsController->stopAll();
    }
    for (int t = 0; t < m_tabModel->count(); ++t) {
        SoundboardGridModel *grid = m_tabModel->gridModelAt(t);
        if (grid) {
            for (SoundTileController *tile : grid->tiles()) {
                if (tile && tile->playbackState() != QMediaPlayer::StoppedState) {
                    tile->stop();
                }
            }
        }
    }
}

void SoundboardProjectController::toggleAudioDucking()
{
    setIsDuckingActive(!m_isDuckingActive);
}

void SoundboardProjectController::applyDuckingToAllTiles()
{
    qreal factor = m_isDuckingActive ? m_duckingVolumeRatio : 1.0;
    for (int t = 0; t < m_tabModel->count(); ++t) {
        SoundboardGridModel *grid = m_tabModel->gridModelAt(t);
        if (grid) {
            for (SoundTileController *tile : grid->tiles()) {
                if (tile) {
                    tile->setDuckingAttenuation(factor);
                }
            }
        }
    }
}

QAudioDevice SoundboardProjectController::currentAudioOutputDevice() const
{
    const auto devices = QMediaDevices::audioOutputs();
    if (!m_outputDeviceName.isEmpty() && m_outputDeviceName != tr("Default System Output")) {
        // 1. Direct exact match on description or ID
        for (const auto &dev : devices) {
            if (dev.description() == m_outputDeviceName || dev.id() == m_outputDeviceName.toUtf8()) {
                return dev;
            }
        }

        // 2. Substring match or virtual managed sink name (e.g. SoundboardSink or Caster_Soundboard)
        for (const auto &dev : devices) {
            const QString desc = dev.description();
            const QByteArray id = dev.id();
            if (id.contains(m_outputDeviceName.toUtf8()) ||
                desc.contains(m_outputDeviceName, Qt::CaseInsensitive) ||
                (m_outputDeviceName.contains("Soundboard", Qt::CaseInsensitive) &&
                 (desc.contains("Caster_Soundboard", Qt::CaseInsensitive) ||
                  desc.contains("Caster Soundboard", Qt::CaseInsensitive) ||
                  desc.contains("Soundboard", Qt::CaseInsensitive)))) {
                return dev;
            }
        }
    }

    return QMediaDevices::defaultAudioOutput();
}

void SoundboardProjectController::applyOutputDeviceToAllTiles()
{
    QAudioDevice targetDevice = currentAudioOutputDevice();

    for (int t = 0; t < m_tabModel->count(); ++t) {
        SoundboardGridModel *grid = m_tabModel->gridModelAt(t);
        if (grid) {
            for (SoundTileController *tile : grid->tiles()) {
                if (tile) {
                    tile->setAudioOutputDevice(targetDevice);
                }
            }
        }
    }
}

void SoundboardProjectController::setupActionsCoordination()
{
    for (int t = 0; t < m_tabModel->count(); ++t) {
        const QString tabName = m_tabModel->tabNameAt(t);
        SoundboardGridModel *grid = m_tabModel->gridModelAt(t);
        if (!grid) continue;
        for (SoundTileController *tile : grid->tiles()) {
            if (!tile) continue;
            tile->setPlaybackCoordinator([this, tabName](SoundTileController *tile, std::function<void()> onReady) {
                if (m_actionsController) {
                    m_actionsController->coordinatePlayback(tile, tabName, onReady);
                } else if (onReady) {
                    onReady();
                }
            });
            tile->setStopCoordinator([this](SoundTileController *tile) {
                if (m_actionsController) {
                    m_actionsController->coordinatePlaybackFinished(tile);
                }
            });
        }
    }
}

QJsonObject SoundboardProjectController::toJson() const
{
    QJsonObject root;
    root["formatVersion"] = 2;

    // Window & UI settings
    QJsonObject windowObj;
    windowObj["width"] = m_windowWidth;
    windowObj["height"] = m_windowHeight;
    windowObj["fullScreen"] = m_isFullScreen;
    windowObj["maximized"] = m_isMaximized;
    windowObj["tileScale"] = m_tileScale;
    root["window"] = windowObj;

    // Audio & general settings
    QJsonObject audioObj;
    audioObj["masterVolume"] = m_masterVolume;
    audioObj["duckingRatio"] = m_duckingVolumeRatio;
    audioObj["outputDevice"] = m_outputDeviceName;
    root["audio"] = audioObj;

    // Active tab
    root["activeTabIndex"] = m_tabModel->currentIndex();

    // Hotkey settings
    root["hotkeyScope"] = m_hotkeyScope;

    // Global action shortcuts
    QJsonObject shortcutsObj;
    shortcutsObj["stopAll"] = m_stopAllHotkey;
    shortcutsObj["ducking"] = m_duckingHotkey;
    root["shortcuts"] = shortcutsObj;

    // Action profiles (Issue #24)
    if (m_actionsController) {
        root["actions"] = m_actionsController->toJson();
    }

    // Tabs & sounds
    root["tabs"] = m_tabModel->toJson();

    return root;
}

bool SoundboardProjectController::fromJson(const QJsonObject &json)
{
    QJsonObject win;
    if (json.contains("window") && json["window"].isObject()) {
        win = json["window"].toObject();
    } else if (json.contains("workspace") && json["workspace"].isObject()) {
        win = json["workspace"].toObject();
    }
    if (!win.isEmpty()) {
        setWindowWidth(win.contains("width") ? win["width"].toInt(1024) : win["windowWidth"].toInt(1024));
        setWindowHeight(win.contains("height") ? win["height"].toInt(720) : win["windowHeight"].toInt(720));
        setIsFullScreen(win.contains("fullScreen") ? win["fullScreen"].toBool(false) : win["isFullScreen"].toBool(false));
        setIsMaximized(win.contains("maximized") ? win["maximized"].toBool(false) : win.contains("isMaximized") ? win["isMaximized"].toBool(false) : false);
        setTileScale(win["tileScale"].toDouble(1.0));
    }

    if (json.contains("audio") && json["audio"].isObject()) {
        QJsonObject audio = json["audio"].toObject();
        setMasterVolume(audio["masterVolume"].toDouble(1.0));
        setDuckingVolumeRatio(audio["duckingRatio"].toDouble(0.25));
        setOutputDeviceName(audio["outputDevice"].toString());
    }

    if (json.contains("hotkeyScope")) {
        setHotkeyScope(json["hotkeyScope"].toInt(TabScoped));
    } else {
        setHotkeyScope(TabScoped);
    }

    if (json.contains("shortcuts") && json["shortcuts"].isObject()) {
        QJsonObject shortcuts = json["shortcuts"].toObject();
        setStopAllHotkey(shortcuts["stopAll"].toString("Space"));
        setDuckingHotkey(shortcuts["ducking"].toString("Ctrl+D"));
    } else {
        setStopAllHotkey("Space");
        setDuckingHotkey("Ctrl+D");
    }

    if (json.contains("actions") && json["actions"].isArray()) {
        if (m_actionsController) {
            m_actionsController->fromJson(json["actions"].toArray());
        }
    } else if (m_actionsController) {
        m_actionsController->fromJson(QJsonArray());
    }

    if (json.contains("tabs") && json["tabs"].isArray()) {
        m_tabModel->fromJson(json["tabs"].toArray());
    }

    if (json.contains("activeTabIndex")) {
        m_tabModel->setCurrentIndex(json["activeTabIndex"].toInt(0));
    }

    applyOutputDeviceToAllTiles();
    applyDuckingToAllTiles();
    setupActionsCoordination();

    return true;
}

void SoundboardProjectController::copyToClipboard(const QString &text)
{
    if (auto clip = QGuiApplication::clipboard()) {
        clip->setText(text);
    }
}

