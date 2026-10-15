#include "HotkeyManager.h"
#include "SoundTileController.h"

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
