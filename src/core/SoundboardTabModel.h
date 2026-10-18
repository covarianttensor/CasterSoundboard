#ifndef SOUNDBOARDTABMODEL_H
#define SOUNDBOARDTABMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include "SoundboardGridModel.h"

struct SoundboardTabItem {
    QString id;
    QString name;
    QString hotkey;
    SoundboardGridModel *gridModel{nullptr};
};

class SoundboardTabModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(SoundboardGridModel* currentGridModel READ currentGridModel NOTIFY currentGridModelChanged)

public:
    enum TabRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        GridModelRole,
        HotkeyRole
    };
    Q_ENUM(TabRoles)

    explicit SoundboardTabModel(QObject *parent = nullptr);
    ~SoundboardTabModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_tabs.size(); }
    int currentIndex() const { return m_currentIndex; }
    SoundboardGridModel* currentGridModel() const;

    enum ImportOrderMode {
        RootFilesFirst = 0,
        RelativePathOrder = 1
    };
    Q_ENUM(ImportOrderMode)

    enum ImportNamingScheme {
        FilenameOnly = 0,
        FolderPrefix = 1,
        DisambiguateDuplicates = 2
    };
    Q_ENUM(ImportNamingScheme)

    Q_INVOKABLE SoundboardGridModel* addTab(const QString &name = QString());
    Q_INVOKABLE int cloneTab(int index, const QString &customName = QString());
    Q_INVOKABLE int countPlayableFilesInDirectory(const QString &dirPath, bool recursive = true) const;
    Q_INVOKABLE int importDirectory(const QString &dirPath,
                                   bool recursive = true,
                                   const QString &customName = QString(),
                                   int orderMode = 0,
                                   int namingScheme = 2);
    Q_INVOKABLE bool removeTab(int index);
    Q_INVOKABLE bool renameTab(int index, const QString &name);
    Q_INVOKABLE bool setTabHotkey(int index, const QString &hotkey);
    Q_INVOKABLE bool moveTab(int fromIndex, int toIndex);
    Q_INVOKABLE SoundboardGridModel* gridModelAt(int index) const;
    Q_INVOKABLE QString tabNameAt(int index) const;
    Q_INVOKABLE QString tabIdAt(int index) const;
    Q_INVOKABLE QString tabHotkeyAt(int index) const;
    Q_INVOKABLE QString defaultHotkeyForIndex(int index) const;

    // Issue #67 tab cycling navigation
    Q_INVOKABLE void nextTab();
    Q_INVOKABLE void previousTab();
    Q_INVOKABLE void setCurrentIndex(int index);

    void clear();

    // Serialization
    QJsonArray toJson() const;
    bool fromJson(const QJsonArray &array);

signals:
    void countChanged();
    void currentIndexChanged();
    void currentGridModelChanged();
    void tabCloned(const QString &sourceTabName, const QString &newTabName);
    void tabRenamed(const QString &oldName, const QString &newName);
    void tabRemoved(const QString &tabName);

private:
    QVector<SoundboardTabItem> m_tabs;
    int m_currentIndex{-1};
};

#endif // SOUNDBOARDTABMODEL_H
