#include "HotkeyManager.h"
#include "SoundTileController.h"
#include "SoundboardProjectController.h"
#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"

HotkeyManager::HotkeyManager(QObject *parent)
    : QObject(parent)
{
}

HotkeyManager::~HotkeyManager()
{
    clear();
}

void HotkeyManager::setIsMuted(bool mute)
{
    if (m_isMuted == mute)
        return;
    m_isMuted = mute;
    emit isMutedChanged();
}

void HotkeyManager::toggleMute()
{
    setIsMuted(!m_isMuted);
}

void HotkeyManager::registerTile(SoundTileController *tile)
{
    if (!tile || m_registeredTiles.contains(tile))
        return;

    m_registeredTiles.append(tile);
    connect(tile, &SoundTileController::hotkeyChanged, this, &HotkeyManager::onTileHotkeyChanged);
    connect(tile, &QObject::destroyed, this, &HotkeyManager::onTileDestroyed);

    rebuildMap();
}

void HotkeyManager::unregisterTile(SoundTileController *tile)
{
    if (!tile || !m_registeredTiles.contains(tile))
        return;

    m_registeredTiles.removeAll(tile);
    disconnect(tile, &SoundTileController::hotkeyChanged, this, &HotkeyManager::onTileHotkeyChanged);
    disconnect(tile, &QObject::destroyed, this, &HotkeyManager::onTileDestroyed);

    rebuildMap();
}

void HotkeyManager::clear()
{
    for (SoundTileController *tile : m_registeredTiles) {
        if (tile) {
            disconnect(tile, &SoundTileController::hotkeyChanged, this, &HotkeyManager::onTileHotkeyChanged);
            disconnect(tile, &QObject::destroyed, this, &HotkeyManager::onTileDestroyed);
        }
    }
    m_registeredTiles.clear();
    m_keyMap.clear();
}

void HotkeyManager::rescanProject()
{
    clear();
    if (!m_monitoredProject || !m_monitoredProject->tabModel())
        return;

    SoundboardTabModel *tabModel = m_monitoredProject->tabModel();
    for (int i = 0; i < tabModel->count(); ++i) {
        SoundboardGridModel *grid = tabModel->gridModelAt(i);
        if (grid) {
            connect(grid, &QAbstractItemModel::rowsInserted, this, &HotkeyManager::rescanProject, Qt::UniqueConnection);
            connect(grid, &QAbstractItemModel::rowsRemoved, this, &HotkeyManager::rescanProject, Qt::UniqueConnection);
            connect(grid, &QAbstractItemModel::modelReset, this, &HotkeyManager::rescanProject, Qt::UniqueConnection);

            for (SoundTileController *tile : grid->tiles()) {
                registerTile(tile);
            }
        }
    }
}

void HotkeyManager::monitorProject(SoundboardProjectController *project)
{
    if (m_monitoredProject == project)
        return;

    if (m_monitoredProject) {
        disconnect(m_monitoredProject, nullptr, this, nullptr);
        if (m_monitoredProject->tabModel()) {
            disconnect(m_monitoredProject->tabModel(), nullptr, this, nullptr);
        }
    }

    m_monitoredProject = project;
    if (!m_monitoredProject) {
        clear();
        return;
    }

    connect(m_monitoredProject, &SoundboardProjectController::projectLoaded, this, &HotkeyManager::rescanProject);

    if (auto *tabModel = m_monitoredProject->tabModel()) {
        connect(tabModel, &QAbstractItemModel::rowsInserted, this, &HotkeyManager::rescanProject);
        connect(tabModel, &QAbstractItemModel::rowsRemoved, this, &HotkeyManager::rescanProject);
        connect(tabModel, &QAbstractItemModel::modelReset, this, &HotkeyManager::rescanProject);
    }

    rescanProject();
}

void HotkeyManager::onTileHotkeyChanged()
{
    rebuildMap();
}

void HotkeyManager::onTileDestroyed(QObject *obj)
{
    SoundTileController *tile = static_cast<SoundTileController*>(obj);
    m_registeredTiles.removeAll(tile);
    rebuildMap();
}

void HotkeyManager::rebuildMap()
{
    m_keyMap.clear();
    for (SoundTileController *tile : m_registeredTiles) {
        if (tile && !tile->keySequence().isEmpty()) {
            m_keyMap.insert(tile->keySequence(), tile);
        }
    }
}

bool HotkeyManager::handleKeyEvent(QKeyEvent *event)
{
    if (m_isMuted || !event)
        return false;

    // Build the QKeySequence from the key event
    int key = event->key();
    if (key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
        return false; // Modifier keys alone don't trigger sounds
    }

    Qt::KeyboardModifiers modifiers = event->modifiers();
    QKeySequence seq(key | modifiers);

    if (m_keyMap.contains(seq)) {
        SoundTileController *tile = m_keyMap.value(seq);
        if (tile) {
            tile->trigger();
            emit hotkeyTriggered(tile);
            event->accept();
            return true;
        }
    }

    return false;
}

QString HotkeyManager::keySequenceToString(int key, int modifiers) const
{
    if (key == 0 || key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
        return QString();
    }

    int cleanModifiers = 0;
    if (modifiers & Qt::ControlModifier) cleanModifiers |= Qt::ControlModifier;
    if (modifiers & Qt::AltModifier) cleanModifiers |= Qt::AltModifier;
    if (modifiers & Qt::ShiftModifier) cleanModifiers |= Qt::ShiftModifier;
    if (modifiers & Qt::MetaModifier) cleanModifiers |= Qt::MetaModifier;

    QKeySequence seq(key | cleanModifiers);
    return seq.toString(QKeySequence::NativeText);
}

QString HotkeyManager::modifiersToString(int modifiers) const
{
    QStringList parts;
    if (modifiers & Qt::ControlModifier) parts << "Ctrl";
    if (modifiers & Qt::AltModifier) parts << "Alt";
    if (modifiers & Qt::ShiftModifier) parts << "Shift";
    if (modifiers & Qt::MetaModifier) parts << "Meta";
    if (parts.isEmpty()) return QString();
    return parts.join("+") + "+...";
}
