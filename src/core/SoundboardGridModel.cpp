#include "SoundboardGridModel.h"
#include "SoundTileController.h"

SoundboardGridModel::SoundboardGridModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

SoundboardGridModel::~SoundboardGridModel()
{
    clear();
}

int SoundboardGridModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_tiles.size();
}

QVariant SoundboardGridModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tiles.size())
        return QVariant();

    SoundTileController *tile = m_tiles.at(index.row());
    switch (role) {
    case TileRole:
        return QVariant::fromValue(tile);
    case IdRole:
        return tile->id();
    case FilePathRole:
        return tile->filePath();
    case DisplayNameRole:
        return tile->displayName();
    case HotkeyRole:
        return tile->hotkey();
    case VolumeRole:
        return tile->volume();
    case IsLoopedRole:
        return tile->isLooped();
    case TriggerStyleRole:
        return static_cast<int>(tile->triggerStyle());
    case HasCueRegionRole:
        return tile->hasCueRegion();
    case CueBeginMsRole:
        return tile->cueBeginMs();
    case CueEndMsRole:
        return tile->cueEndMs();
    case ColorRole:
        return tile->color();
    case PlaybackStateRole:
        return static_cast<int>(tile->playbackState());
    case PositionMsRole:
        return tile->positionMs();
    case DurationMsRole:
        return tile->durationMs();
    case HasVideoRole:
        return tile->hasVideo();
    default:
        break;
    }

    return QVariant();
}

bool SoundboardGridModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tiles.size())
        return false;

    SoundTileController *tile = m_tiles.at(index.row());
    switch (role) {
    case FilePathRole:
        tile->setFilePath(value.toString());
        return true;
    case DisplayNameRole:
        tile->setDisplayName(value.toString());
        return true;
    case HotkeyRole:
        tile->setHotkey(value.toString());
        return true;
    case VolumeRole:
        tile->setVolume(value.toReal());
        return true;
    case IsLoopedRole:
        tile->setIsLooped(value.toBool());
        return true;
    case TriggerStyleRole:
        tile->setTriggerStyle(static_cast<SoundTileController::TriggerStyle>(value.toInt()));
        return true;
    case HasCueRegionRole:
        tile->setHasCueRegion(value.toBool());
        return true;
    case CueBeginMsRole:
        tile->setCueBeginMs(value.toLongLong());
        return true;
    case CueEndMsRole:
        tile->setCueEndMs(value.toLongLong());
        return true;
    case ColorRole:
        tile->setColor(value.toString());
        return true;
    default:
        break;
    }

    return false;
}

QHash<int, QByteArray> SoundboardGridModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TileRole] = "tile";
    roles[IdRole] = "id";
    roles[FilePathRole] = "filePath";
    roles[DisplayNameRole] = "displayName";
    roles[HotkeyRole] = "hotkey";
    roles[VolumeRole] = "volume";
    roles[IsLoopedRole] = "isLooped";
    roles[TriggerStyleRole] = "triggerStyle";
    roles[HasCueRegionRole] = "hasCueRegion";
    roles[CueBeginMsRole] = "cueBeginMs";
    roles[CueEndMsRole] = "cueEndMs";
    roles[ColorRole] = "color";
    roles[PlaybackStateRole] = "playbackState";
    roles[PositionMsRole] = "positionMs";
    roles[DurationMsRole] = "durationMs";
    roles[HasVideoRole] = "hasVideo";
    return roles;
}

SoundTileController* SoundboardGridModel::tileAt(int row) const
{
    if (row < 0 || row >= m_tiles.size())
        return nullptr;
    return m_tiles.at(row);
}

SoundTileController* SoundboardGridModel::appendTile()
{
    return insertTile(m_tiles.size());
}

SoundTileController* SoundboardGridModel::insertTile(int row)
{
    row = qBound(0, row, m_tiles.size());

    beginInsertRows(QModelIndex(), row, row);
    SoundTileController *tile = new SoundTileController(this);
    setupTileConnections(tile);
    m_tiles.insert(row, tile);
    endInsertRows();

    emit countChanged();
    return tile;
}

bool SoundboardGridModel::removeTile(int row)
{
    if (row < 0 || row >= m_tiles.size())
        return false;

    beginRemoveRows(QModelIndex(), row, row);
    SoundTileController *tile = m_tiles.takeAt(row);
    tile->deleteLater();
    endRemoveRows();

    emit countChanged();
    return true;
}

bool SoundboardGridModel::moveTile(int fromRow, int toRow)
{
    if (fromRow < 0 || fromRow >= m_tiles.size() || toRow < 0 || toRow >= m_tiles.size() || fromRow == toRow)
        return false;

    int destinationRow = toRow > fromRow ? toRow + 1 : toRow;
    if (!beginMoveRows(QModelIndex(), fromRow, fromRow, QModelIndex(), destinationRow))
        return false;

    SoundTileController *tile = m_tiles.takeAt(fromRow);
    m_tiles.insert(toRow, tile);
    endMoveRows();

    return true;
}

void SoundboardGridModel::clear()
{
    if (m_tiles.isEmpty())
        return;

    beginResetModel();
    qDeleteAll(m_tiles);
    m_tiles.clear();
    endResetModel();

    emit countChanged();
}

void SoundboardGridModel::setupTileConnections(SoundTileController *tile)
{
    auto emitRowChange = [this, tile](int role) {
        int r = m_tiles.indexOf(tile);
        if (r >= 0) {
            QModelIndex idx = index(r, 0);
            emit dataChanged(idx, idx, {role});
        }
    };

    connect(tile, &SoundTileController::filePathChanged, this, [=]() { emitRowChange(FilePathRole); });
    connect(tile, &SoundTileController::displayNameChanged, this, [=]() { emitRowChange(DisplayNameRole); });
    connect(tile, &SoundTileController::hotkeyChanged, this, [=]() { emitRowChange(HotkeyRole); });
    connect(tile, &SoundTileController::volumeChanged, this, [=]() { emitRowChange(VolumeRole); });
    connect(tile, &SoundTileController::isLoopedChanged, this, [=]() { emitRowChange(IsLoopedRole); });
    connect(tile, &SoundTileController::triggerStyleChanged, this, [=]() { emitRowChange(TriggerStyleRole); });
    connect(tile, &SoundTileController::hasCueRegionChanged, this, [=]() { emitRowChange(HasCueRegionRole); });
    connect(tile, &SoundTileController::cueBeginMsChanged, this, [=]() { emitRowChange(CueBeginMsRole); });
    connect(tile, &SoundTileController::cueEndMsChanged, this, [=]() { emitRowChange(CueEndMsRole); });
    connect(tile, &SoundTileController::colorChanged, this, [=]() { emitRowChange(ColorRole); });
    connect(tile, &SoundTileController::playbackStateChanged, this, [=]() { emitRowChange(PlaybackStateRole); });
    connect(tile, &SoundTileController::positionMsChanged, this, [=]() { emitRowChange(PositionMsRole); });
    connect(tile, &SoundTileController::durationMsChanged, this, [=]() { emitRowChange(DurationMsRole); });
    connect(tile, &SoundTileController::hasVideoChanged, this, [=]() { emitRowChange(HasVideoRole); });
}

QJsonArray SoundboardGridModel::toJson() const
{
    QJsonArray array;
    for (const SoundTileController *tile : m_tiles) {
        if (tile) {
            array.append(tile->toJson());
        }
    }
    return array;
}

bool SoundboardGridModel::fromJson(const QJsonArray &array)
{
    clear();

    if (array.isEmpty())
        return true;

    beginInsertRows(QModelIndex(), 0, array.size() - 1);
    for (const QJsonValue &val : array) {
        if (val.isObject()) {
            SoundTileController *tile = new SoundTileController(this);
            setupTileConnections(tile);
            tile->fromJson(val.toObject());
            m_tiles.append(tile);
        }
    }
    endInsertRows();

    emit countChanged();
    return true;
}
