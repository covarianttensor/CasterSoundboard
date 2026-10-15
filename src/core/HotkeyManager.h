#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H

#include <QObject>
#include <QKeySequence>
#include <QMap>
#include <QKeyEvent>

class SoundTileController;

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

    // In-app key dispatch (returns true if an assigned hotkey matched and triggered)
    Q_INVOKABLE bool handleKeyEvent(QKeyEvent *event);

public slots:
    void toggleMute();

signals:
    void isMutedChanged();
    void hotkeyTriggered(SoundTileController *tile);

private slots:
    void onTileHotkeyChanged();
    void onTileDestroyed(QObject *obj);

private:
    void rebuildMap();

    bool m_isMuted{false};
    QList<SoundTileController*> m_registeredTiles;
    QMap<QKeySequence, SoundTileController*> m_keyMap;
};

#endif // HOTKEYMANAGER_H
