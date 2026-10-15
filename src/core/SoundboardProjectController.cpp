#include "SoundboardProjectController.h"
#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"
#include "SoundTileController.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

SoundboardProjectController::SoundboardProjectController(QObject *parent)
    : QObject(parent)
{
    m_tabModel = new SoundboardTabModel(this);
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

    emit filePathChanged();
    emit isDirtyChanged();
    emit isDuckingActiveChanged();
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

void SoundboardProjectController::applyOutputDeviceToAllTiles()
{
    if (m_outputDeviceName.isEmpty())
        return;

    QAudioDevice targetDevice;
    const auto devices = QMediaDevices::audioOutputs();
    for (const auto &dev : devices) {
        if (dev.description() == m_outputDeviceName) {
            targetDevice = dev;
            break;
        }
    }

    if (targetDevice.isNull())
        return;

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

QJsonObject SoundboardProjectController::toJson() const
{
    QJsonObject root;
    root["formatVersion"] = 2;

    // Window & UI settings
    QJsonObject windowObj;
    windowObj["width"] = m_windowWidth;
    windowObj["height"] = m_windowHeight;
    windowObj["fullScreen"] = m_isFullScreen;
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

    // Tabs & sounds
    root["tabs"] = m_tabModel->toJson();

    return root;
}

bool SoundboardProjectController::fromJson(const QJsonObject &json)
{
    if (json.contains("window") && json["window"].isObject()) {
        QJsonObject win = json["window"].toObject();
        setWindowWidth(win["width"].toInt(1024));
        setWindowHeight(win["height"].toInt(720));
        setIsFullScreen(win["fullScreen"].toBool(false));
        setTileScale(win["tileScale"].toDouble(1.0));
    }

    if (json.contains("audio") && json["audio"].isObject()) {
        QJsonObject audio = json["audio"].toObject();
        setMasterVolume(audio["masterVolume"].toDouble(1.0));
        setDuckingVolumeRatio(audio["duckingRatio"].toDouble(0.25));
        setOutputDeviceName(audio["outputDevice"].toString());
    }

    if (json.contains("tabs") && json["tabs"].isArray()) {
        m_tabModel->fromJson(json["tabs"].toArray());
    }

    if (json.contains("activeTabIndex")) {
        m_tabModel->setCurrentIndex(json["activeTabIndex"].toInt(0));
    }

    applyOutputDeviceToAllTiles();
    applyDuckingToAllTiles();

    return true;
}
