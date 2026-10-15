#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H

#include <QObject>
#include <QKeySequence>
#include <QMap>
#include <QKeyEvent>

#include "SoundTileController.h"
#include "SoundboardProjectController.h"

class HotkeyManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isMuted READ isMuted WRITE setIsMuted NOTIFY isMutedChanged)

public:
    explicit HotkeyManager(QObject *parent = nullptr);
    ~HotkeyManager() override;

    bool isMuted() const { return m_isMuted; }
    void setIsMuted(bool mute);

    Q_INVOKABLE void registerTile(SoundTileController *tile);
    Q_INVOKABLE void unregisterTile(SoundTileController *tile);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void monitorProject(SoundboardProjectController *project);

    // In-app key dispatch (returns true if an assigned hotkey matched and triggered)
    Q_INVOKABLE bool handleKeyEvent(QKeyEvent *event);

    // Helpers to format keyboard shortcuts for UI prompts
    Q_INVOKABLE QString keySequenceToString(int key, int modifiers) const;
    Q_INVOKABLE QString modifiersToString(int modifiers) const;

public slots:
    void toggleMute();
    void rescanProject();

signals:
    void isMutedChanged();
    void hotkeyTriggered(SoundTileController *tile);

private slots:
    void onTileHotkeyChanged();
    void onTileDestroyed(QObject *obj);

private:
    void rebuildMap();

    bool m_isMuted{false};
    SoundboardProjectController *m_monitoredProject{nullptr};
    QList<SoundTileController*> m_registeredTiles;
    QMap<QKeySequence, SoundTileController*> m_keyMap;
};

#endif // HOTKEYMANAGER_H
