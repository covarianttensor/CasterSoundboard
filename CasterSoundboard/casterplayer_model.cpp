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
    QHash<int, QByteArray> names;
    names[IsInPlayerModeRole] = "isInPlayerMode";
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
