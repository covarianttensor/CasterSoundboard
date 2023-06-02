#include "casterplayer_controller.h"

CasterPlayerController::CasterPlayerController(QObject *parent) : QObject(parent)
{
    // Add initial player
    mItems.append({
        false, // IsInPlayerMode
        false, // IsLooped
        1.0,   // Volume
        false, // IsPlayRegionEnabled
        0,     // PlayRegionBegin
        0,     // PlayRegionEnd
        0     // Trigger Style
    });
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
    // Adds player with default settings
    emit preItemAppend();
    CasterPlayerItem item;
    item.isInPlayerMode = false;
    item.isLooped = false;
    item.volume = 1.0;
    item.isPlayRegionEnabled = false;
    item.playRegionBegin = 0;
    item.playRegionEnd = 0;
    item.triggerStyle = 0;
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
