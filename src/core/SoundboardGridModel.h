#ifndef SOUNDBOARDGRIDMODEL_H
#define SOUNDBOARDGRIDMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>

class SoundTileController;

class SoundboardGridModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum SoundboardRoles {
        TileRole = Qt::UserRole + 1,
        IdRole,
        FilePathRole,
        DisplayNameRole,
        HotkeyRole,
        VolumeRole,
        IsLoopedRole,
        TriggerStyleRole,
        HasCueRegionRole,
        CueBeginMsRole,
        CueEndMsRole,
        ColorRole,
        PlaybackStateRole,
        PositionMsRole,
        DurationMsRole,
        HasVideoRole
    };
    Q_ENUM(SoundboardRoles)

    explicit SoundboardGridModel(QObject *parent = nullptr);
    ~SoundboardGridModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_tiles.size(); }

    Q_INVOKABLE SoundTileController* tileAt(int row) const;
    Q_INVOKABLE SoundTileController* appendTile();
    Q_INVOKABLE SoundTileController* insertTile(int row);
    Q_INVOKABLE bool removeTile(int row);
    Q_INVOKABLE bool moveTile(int fromRow, int toRow);
    Q_INVOKABLE void clear();

    QVector<SoundTileController*> tiles() const { return m_tiles; }

    // Serialization
    QJsonArray toJson() const;
    bool fromJson(const QJsonArray &array);

signals:
    void countChanged();
    void tileTriggered(SoundTileController *tile);

private:
    void setupTileConnections(SoundTileController *tile);

    QVector<SoundTileController*> m_tiles;
};

#endif // SOUNDBOARDGRIDMODEL_H
