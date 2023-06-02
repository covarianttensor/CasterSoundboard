#include "casterplayer_model.h"

#include "casterplayer_controller.h"

CasterPlayerModel::CasterPlayerModel(QObject *parent)
    : QAbstractListModel(parent)
    , mList(nullptr)
{
}

int CasterPlayerModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid() || !mList)
        return 0;

    return mList->items().size();
}

QVariant CasterPlayerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !mList)
        return QVariant();

    const CasterPlayerItem item = mList->items().at(index.row());
    switch (role) {
    case IsInPlayerModeRole:
        return QVariant(item.isInPlayerMode);
    case IsLoopedRole:
        return QVariant(item.isLooped);
    case VolumeRole:
        return QVariant(item.volume);
    case IsPlayingRegionEnabledRole:
        return QVariant(item.isPlayRegionEnabled);
    case PlayRegionBeginRole:
        return QVariant(item.playRegionBegin);
    case PlayRegionEndRole:
        return QVariant(item.playRegionEnd);
    case TriggerStyleRole:
        return QVariant(item.triggerStyle);
    }

    return QVariant();
}

bool CasterPlayerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!mList)
        return false;

    CasterPlayerItem item = mList->items().at(index.row());
    switch (role) {
    case IsInPlayerModeRole:
        item.isInPlayerMode = value.toBool();
        break;
    case IsLoopedRole:
        item.isLooped = value.toBool();
        break;
    case VolumeRole:
        item.volume = value.toDouble();
        break;
    case IsPlayingRegionEnabledRole:
        item.isPlayRegionEnabled = value.toBool();
        break;
    case PlayRegionBeginRole:
        item.playRegionBegin = value.toInt();
        break;
    case PlayRegionEndRole:
        item.playRegionEnd = value.toInt();
        break;
    case TriggerStyleRole:
        item.triggerStyle = value.toInt();
    }

    if (mList->setItemAt(index.row(), item)) {
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags CasterPlayerModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable;
}

/*
 * Define the names of the model properties to be used in QML, by
 * associating the names with their respective role in the model.
*/
QHash<int, QByteArray> CasterPlayerModel::roleNames() const
{
    // Set QML property names
    QHash<int, QByteArray> names;
    names[IsInPlayerModeRole] = "isInPlayerMode";
    names[IsLoopedRole] = "isLooped";
    names[VolumeRole] = "volume";
    names[IsPlayingRegionEnabledRole] = "isPlayRegionEnabled";
    names[PlayRegionBeginRole] = "playRegionBegin";
    names[PlayRegionEndRole] = "playRegionEnd";
    names[TriggerStyleRole] = "triggerStyle";
    return names;
}

CasterPlayerController *CasterPlayerModel::list() const
{
    return mList;
}

void CasterPlayerModel::setList(CasterPlayerController *list)
{
    beginResetModel();

    if(mList)
        mList->disconnect(this);

    mList = list;

    if (mList) {
        connect(mList, &CasterPlayerController::preItemAppend, this, [=]() {
            const int index = mList->items().size();
            beginInsertRows(QModelIndex(), index, index);
        });
        connect(mList, &CasterPlayerController::postItemAppend, this, [=]() {
            endInsertRows();
        });

        connect(mList, &CasterPlayerController::preItemRemoved, this, [=](int index) {
            beginRemoveRows(QModelIndex(), index, index);
        });
        connect(mList, &CasterPlayerController::postItemRemoved, this, [=]() {
            endRemoveRows();
        });
    }

    endResetModel();
}
