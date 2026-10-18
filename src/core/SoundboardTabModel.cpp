#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"
#include "SoundTileController.h"
#include <QUuid>
#include <QSet>
#include <QDir>
#include <QDirIterator>
#include <QCollator>
#include <QUrl>
#include <QHash>
#include <algorithm>

namespace {
static const QStringList kSupportedMediaFilters = {
    "*.wav", "*.mp3", "*.ogg", "*.flac", "*.aac", "*.m4a", "*.opus",
    "*.mp4", "*.mkv", "*.webm", "*.avi"
};

static QString cleanPathFromUrl(const QString &rawPath)
{
    QString path = rawPath.trimmed();
    if (path.startsWith("file://")) {
        path = QUrl(rawPath).toLocalFile();
        if (path.isEmpty()) {
            path = rawPath.mid(7);
        }
    }
    return QDir::cleanPath(path);
}
}

SoundboardTabModel::SoundboardTabModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

SoundboardTabModel::~SoundboardTabModel()
{
    clear();
}

int SoundboardTabModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_tabs.size();
}

QVariant SoundboardTabModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tabs.size())
        return QVariant();

    const SoundboardTabItem &tab = m_tabs.at(index.row());
    switch (role) {
    case IdRole:
        return tab.id;
    case NameRole:
        return tab.name;
    case GridModelRole:
        return QVariant::fromValue(tab.gridModel);
    case HotkeyRole:
        return tab.hotkey;
    default:
        break;
    }

    return QVariant();
}

bool SoundboardTabModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tabs.size())
        return false;

    if (role == NameRole) {
        return renameTab(index.row(), value.toString());
    }
    if (role == HotkeyRole) {
        return setTabHotkey(index.row(), value.toString());
    }

    return false;
}

QHash<int, QByteArray> SoundboardTabModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[GridModelRole] = "gridModel";
    roles[HotkeyRole] = "hotkey";
    return roles;
}

SoundboardGridModel* SoundboardTabModel::currentGridModel() const
{
    return gridModelAt(m_currentIndex);
}

SoundboardGridModel* SoundboardTabModel::gridModelAt(int index) const
{
    if (index < 0 || index >= m_tabs.size())
        return nullptr;
    return m_tabs.at(index).gridModel;
}

QString SoundboardTabModel::tabNameAt(int index) const
{
    if (index < 0 || index >= m_tabs.size())
        return QString();
    return m_tabs.at(index).name;
}

QString SoundboardTabModel::tabIdAt(int index) const
{
    if (index < 0 || index >= m_tabs.size())
        return QString();
    return m_tabs.at(index).id;
}

SoundboardGridModel* SoundboardTabModel::addTab(const QString &name)
{
    int row = m_tabs.size();
    QString finalName = name.isEmpty() ? QString("Sound Set %1").arg(row + 1) : name;
    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);

    beginInsertRows(QModelIndex(), row, row);
    SoundboardTabItem item;
    item.id = id;
    item.name = finalName;
    item.gridModel = new SoundboardGridModel(this);
    m_tabs.append(item);
    endInsertRows();

    emit countChanged();

    if (m_tabs.size() == 1) {
        setCurrentIndex(0);
    }

    return item.gridModel;
}

int SoundboardTabModel::cloneTab(int index, const QString &customName)
{
    if (index < 0 || index >= m_tabs.size())
        return -1;

    const SoundboardTabItem &sourceTab = m_tabs.at(index);
    const QString sourceName = sourceTab.name;

    QString newName = customName.trimmed();
    if (newName.isEmpty()) {
        newName = QString("%1 (Copy)").arg(sourceName);
        QSet<QString> existingNames;
        for (const auto &t : m_tabs) {
            existingNames.insert(t.name);
        }
        int copyNum = 2;
        while (existingNames.contains(newName)) {
            newName = QString("%1 (Copy %2)").arg(sourceName).arg(copyNum++);
        }
    }

    int insertIndex = index + 1;
    QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    beginInsertRows(QModelIndex(), insertIndex, insertIndex);

    SoundboardTabItem newItem;
    newItem.id = newId;
    newItem.name = newName;
    newItem.hotkey = QString(); // Reset tab hotkey to avoid duplicate tab switch hotkeys
    newItem.gridModel = new SoundboardGridModel(this);

    if (sourceTab.gridModel) {
        QJsonArray sourceTiles = sourceTab.gridModel->toJson();
        QJsonArray clonedTiles;
        for (const QJsonValue &val : sourceTiles) {
            if (val.isObject()) {
                QJsonObject tileObj = val.toObject();
                // Assign a fresh, unique UUID to each cloned tile
                tileObj["id"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
                clonedTiles.append(tileObj);
            }
        }
        newItem.gridModel->fromJson(clonedTiles);
    }

    m_tabs.insert(insertIndex, newItem);
    endInsertRows();

    emit countChanged();
    emit tabCloned(sourceName, newName);

    // Switch selection to the cloned tab
    m_currentIndex = insertIndex;
    emit currentIndexChanged();
    emit currentGridModelChanged();

    return insertIndex;
}

int SoundboardTabModel::countPlayableFilesInDirectory(const QString &dirPath, bool recursive) const
{
    QString clean = cleanPathFromUrl(dirPath);
    if (clean.isEmpty() || !QDir(clean).exists())
        return 0;

    int count = 0;
    if (recursive) {
        QDirIterator it(clean, kSupportedMediaFilters, QDir::Files | QDir::Readable,
                        QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
        while (it.hasNext()) {
            it.next();
            count++;
        }
    } else {
        QDir dir(clean);
        count = dir.entryList(kSupportedMediaFilters, QDir::Files | QDir::Readable).size();
    }
    return count;
}

int SoundboardTabModel::importDirectory(const QString &dirPath,
                                       bool recursive,
                                       const QString &customName,
                                       int orderMode,
                                       int namingScheme)
{
    QString clean = cleanPathFromUrl(dirPath);
    if (clean.isEmpty() || !QDir(clean).exists())
        return -1;

    QDir rootDir(clean);
    QString rootCanonical = rootDir.canonicalPath();

    QFileInfoList foundFiles;
    if (recursive) {
        QDirIterator it(clean, kSupportedMediaFilters, QDir::Files | QDir::Readable,
                        QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
        while (it.hasNext()) {
            it.next();
            foundFiles.append(it.fileInfo());
        }
    } else {
        foundFiles = rootDir.entryInfoList(kSupportedMediaFilters, QDir::Files | QDir::Readable);
    }

    if (foundFiles.isEmpty())
        return -1;

    QCollator collator;
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);

    // Sorting according to orderMode
    if (orderMode == 0) { // RootFilesFirst
        QFileInfoList rootFiles;
        QFileInfoList nestedFiles;
        for (const auto &fi : foundFiles) {
            if (fi.dir().canonicalPath() == rootCanonical) {
                rootFiles.append(fi);
            } else {
                nestedFiles.append(fi);
            }
        }

        std::sort(rootFiles.begin(), rootFiles.end(), [&collator](const QFileInfo &a, const QFileInfo &b) {
            return collator.compare(a.fileName(), b.fileName()) < 0;
        });

        std::sort(nestedFiles.begin(), nestedFiles.end(), [&collator, &rootDir](const QFileInfo &a, const QFileInfo &b) {
            QString relA = rootDir.relativeFilePath(a.absoluteFilePath());
            QString relB = rootDir.relativeFilePath(b.absoluteFilePath());
            return collator.compare(relA, relB) < 0;
        });

        foundFiles = rootFiles;
        foundFiles.append(nestedFiles);
    } else { // RelativePathOrder
        std::sort(foundFiles.begin(), foundFiles.end(), [&collator, &rootDir](const QFileInfo &a, const QFileInfo &b) {
            QString relA = rootDir.relativeFilePath(a.absoluteFilePath());
            QString relB = rootDir.relativeFilePath(b.absoluteFilePath());
            return collator.compare(relA, relB) < 0;
        });
    }

    // Determine soundboard tab name
    QString finalTabName = customName.trimmed();
    if (finalTabName.isEmpty()) {
        finalTabName = rootDir.dirName();
        if (finalTabName.isEmpty()) {
            finalTabName = tr("Imported Soundboard");
        }
    }

    // Ensure tab name uniqueness
    QSet<QString> existingNames;
    for (const auto &t : m_tabs) {
        existingNames.insert(t.name);
    }
    QString candidateName = finalTabName;
    int copyNum = 2;
    while (existingNames.contains(candidateName)) {
        candidateName = QString("%1 (%2)").arg(finalTabName).arg(copyNum++);
    }
    finalTabName = candidateName;

    // Track duplicate base names if namingScheme == DisambiguateDuplicates (2)
    QHash<QString, int> nameCounts;
    if (namingScheme == 2) {
        for (const auto &fi : foundFiles) {
            nameCounts[fi.completeBaseName()]++;
        }
    }

    // Create the new soundboard tab
    SoundboardGridModel *newGrid = addTab(finalTabName);
    if (!newGrid)
        return -1;

    for (const auto &fi : foundFiles) {
        QString displayName = fi.completeBaseName();
        bool isNested = (fi.dir().canonicalPath() != rootCanonical);

        if (isNested) {
            QString relDir = rootDir.relativeFilePath(fi.dir().absolutePath());
            if (namingScheme == 1) { // FolderPrefix
                displayName = QString("%1 / %2").arg(relDir, fi.completeBaseName());
            } else if (namingScheme == 2) { // DisambiguateDuplicates
                if (nameCounts.value(fi.completeBaseName(), 0) > 1) {
                    displayName = QString("%1 / %2").arg(relDir, fi.completeBaseName());
                }
            }
        }

        SoundTileController *tile = newGrid->appendTile();
        if (tile) {
            tile->setFilePath(fi.absoluteFilePath());
            tile->setDisplayName(displayName);
        }
    }

    int newIndex = m_tabs.size() - 1;
    setCurrentIndex(newIndex);
    return newIndex;
}

bool SoundboardTabModel::removeTab(int index)
{
    if (index < 0 || index >= m_tabs.size() || m_tabs.size() <= 1)
        return false; // Prevent removing the last remaining tab

    QString removedName = m_tabs.at(index).name;

    beginRemoveRows(QModelIndex(), index, index);
    SoundboardTabItem item = m_tabs.takeAt(index);
    if (item.gridModel) {
        item.gridModel->deleteLater();
    }
    endRemoveRows();

    emit countChanged();
    emit tabRemoved(removedName);

    if (m_currentIndex >= m_tabs.size()) {
        setCurrentIndex(m_tabs.size() - 1);
    } else {
        emit currentGridModelChanged();
    }

    return true;
}

bool SoundboardTabModel::renameTab(int index, const QString &name)
{
    if (index < 0 || index >= m_tabs.size() || name.trimmed().isEmpty())
        return false;

    if (m_tabs[index].name == name)
        return true;

    QString oldName = m_tabs[index].name;
    m_tabs[index].name = name;
    QModelIndex idx = this->index(index, 0);
    emit dataChanged(idx, idx, {NameRole});
    emit tabRenamed(oldName, name);
    return true;
}

QString SoundboardTabModel::tabHotkeyAt(int index) const
{
    if (index < 0 || index >= m_tabs.size())
        return QString();
    return m_tabs.at(index).hotkey;
}

bool SoundboardTabModel::setTabHotkey(int index, const QString &hotkey)
{
    if (index < 0 || index >= m_tabs.size())
        return false;

    QString clean = hotkey.trimmed();
    if (m_tabs[index].hotkey == clean)
        return true;

    m_tabs[index].hotkey = clean;
    QModelIndex idx = this->index(index, 0);
    emit dataChanged(idx, idx, {HotkeyRole});
    return true;
}

QString SoundboardTabModel::defaultHotkeyForIndex(int index) const
{
    if (index >= 0 && index < 9) {
        return QString("Alt+%1").arg(index + 1);
    }
    return QString();
}

bool SoundboardTabModel::moveTab(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= m_tabs.size() || toIndex < 0 || toIndex >= m_tabs.size() || fromIndex == toIndex)
        return false;

    int destinationRow = toIndex > fromIndex ? toIndex + 1 : toIndex;
    if (!beginMoveRows(QModelIndex(), fromIndex, fromIndex, QModelIndex(), destinationRow))
        return false;

    SoundboardTabItem item = m_tabs.takeAt(fromIndex);
    m_tabs.insert(toIndex, item);
    endMoveRows();

    if (m_currentIndex == fromIndex) {
        setCurrentIndex(toIndex);
    } else if (fromIndex < m_currentIndex && toIndex >= m_currentIndex) {
        setCurrentIndex(m_currentIndex - 1);
    } else if (fromIndex > m_currentIndex && toIndex <= m_currentIndex) {
        setCurrentIndex(m_currentIndex + 1);
    }

    return true;
}

void SoundboardTabModel::setCurrentIndex(int index)
{
    if (m_tabs.isEmpty()) {
        if (m_currentIndex != -1) {
            m_currentIndex = -1;
            emit currentIndexChanged();
            emit currentGridModelChanged();
        }
        return;
    }

    index = qBound(0, index, m_tabs.size() - 1);
    if (m_currentIndex == index)
        return;

    m_currentIndex = index;
    emit currentIndexChanged();
    emit currentGridModelChanged();
}

void SoundboardTabModel::nextTab()
{
    if (m_tabs.size() <= 1)
        return;
    int next = (m_currentIndex + 1) % m_tabs.size();
    setCurrentIndex(next);
}

void SoundboardTabModel::previousTab()
{
    if (m_tabs.size() <= 1)
        return;
    int prev = (m_currentIndex - 1 + m_tabs.size()) % m_tabs.size();
    setCurrentIndex(prev);
}

void SoundboardTabModel::clear()
{
    if (m_tabs.isEmpty())
        return;

    beginResetModel();
    for (const SoundboardTabItem &item : m_tabs) {
        if (item.gridModel) {
            item.gridModel->deleteLater();
        }
    }
    m_tabs.clear();
    m_currentIndex = -1;
    endResetModel();

    emit countChanged();
    emit currentIndexChanged();
    emit currentGridModelChanged();
}

QJsonArray SoundboardTabModel::toJson() const
{
    QJsonArray array;
    for (const SoundboardTabItem &tab : m_tabs) {
        QJsonObject obj;
        obj["id"] = tab.id;
        obj["name"] = tab.name;
        obj["hotkey"] = tab.hotkey;
        if (tab.gridModel) {
            obj["sounds"] = tab.gridModel->toJson();
        }
        array.append(obj);
    }
    return array;
}

bool SoundboardTabModel::fromJson(const QJsonArray &array)
{
    clear();

    if (array.isEmpty()) {
        addTab();
        return true;
    }

    beginInsertRows(QModelIndex(), 0, array.size() - 1);
    for (const QJsonValue &val : array) {
        if (val.isObject()) {
            QJsonObject obj = val.toObject();
            SoundboardTabItem item;
            item.id = obj["id"].toString(QUuid::createUuid().toString(QUuid::WithoutBraces));
            item.name = obj["name"].toString("Sound Set");
            item.hotkey = obj.value("hotkey").toString();
            item.gridModel = new SoundboardGridModel(this);
            if (obj.contains("sounds") && obj["sounds"].isArray()) {
                item.gridModel->fromJson(obj["sounds"].toArray());
            }
            m_tabs.append(item);
        }
    }
    endInsertRows();

    emit countChanged();
    setCurrentIndex(0);
    return true;
}
