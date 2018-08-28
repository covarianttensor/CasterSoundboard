#include "casterplayer_controller.h"

CasterPlayerController::CasterPlayerController(QObject *parent) : QObject(parent)
{
    mItems.append({ false });
}

QVector<CasterPlayerItem> CasterPlayerController::items() const
{
    return mItems;
}

bool CasterPlayerController::setItemAt(int index, const CasterPlayerItem &item)
{
    //Check for out of range index
    if (index < 0 || mItems.size() <= index)
        return false;

    // Set Item
    mItems[index] = item;
    return true;
}

void CasterPlayerController::appendItem()
{
    // Adds player
    emit preItemAppend();
    CasterPlayerItem item;
    item.isInPlayerMode = false;
    mItems.append(item);
    emit postItemAppend();
}

void CasterPlayerController::removeItemAt(int index)
{
    //Check if valid index
    if (0 <= index && index < mItems.size()) {
        emit preItemRemoved(index);

        mItems.removeAt(index);

        emit postItemRemoved();
    }
}

void CasterPlayerController::removeAllItems()
{
    for (int i = 0; i < mItems.size(); ) {
        emit preItemRemoved(i);

        mItems.removeAt(i);

        emit postItemRemoved();
    }
}
