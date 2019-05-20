#ifndef CASTERPLAYERMODEL_H
#define CASTERPLAYERMODEL_H

#include <QAbstractListModel>

class CasterPlayerController;

class CasterPlayerModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(CasterPlayerController *list READ list WRITE setList)

public:
    explicit CasterPlayerModel(QObject *parent = nullptr);

    enum ModelRoles {
        IsInPlayerModeRole = Qt::UserRole,
        IsLoopedRole,
        VolumeRole,
        IsPlayingRegionEnabledRole,
        PlayRegionBeginRole,
        PlayRegionEndRole,
        TriggerStyleRole
    };
    Q_ENUM(ModelRoles)
    enum TriggerStyle {
        PlayPauseTriggerStyle = 0,
        PlayStopTriggerStyle,
        PlayAgainTriggerStyle,
        SoundEffectTriggerStyle
    };
    Q_ENUM(TriggerStyle)

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    virtual QHash<int, QByteArray> roleNames() const override;

    CasterPlayerController *list() const;

    void setList(CasterPlayerController *list);

private:
    CasterPlayerController *mList;
};

#endif // CASTERPLAYERMODEL_H
