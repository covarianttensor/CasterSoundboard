#ifndef CASTERPLAYER_CONTROLLER_H
#define CASTERPLAYER_CONTROLLER_H

#include <QObject>
#include <QVector>


struct CasterPlayerItem {
    bool isInPlayerMode;
    bool isLooped;
    double volume;
    bool isPlayRegionEnabled;
    int playRegionBegin;
    int playRegionEnd;
    int triggerStyle;
};


class CasterPlayerController : public QObject
{
    Q_OBJECT
public:
    explicit CasterPlayerController(QObject *parent = nullptr);

    QVector<CasterPlayerItem> items() const;

    bool setItemAt(int index, const CasterPlayerItem &item);

signals:
    void preItemAppend();
    void postItemAppend();

    void preItemRemoved(int index);
    void postItemRemoved();

public slots:
    void appendItem();
    void removeItemAt(int index);
    void removeAllItems();

private:
    QVector<CasterPlayerItem> mItems;
};

#endif // CASTERPLAYER_CONTROLLER_H
