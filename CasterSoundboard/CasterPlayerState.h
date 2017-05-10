#ifndef CASTERPLAYERSTATE_H
#define CASTERPLAYERSTATE_H
#include <QDataStream>
#include <QString>

//forward declarations
class QString;

class CasterPlayerState
{
public:
    //Constructor
    CasterPlayerState();

    // Properties
    QString *filePath;
    int volume;
    int startTime;// Milliseconds
    int stopTime;// Milliseconds
    bool loop;
    bool timeSet;
    int slider_H_Value;
    int slider_S_Value;
    int slider_L_Value;
    QString *PlayerBackgroundCSS;
    bool backgroundCSSChanged;
};

//Operator Overloading
QDataStream &operator<<(QDataStream &ds, const CasterPlayerState &cps);
QDataStream &operator>>(QDataStream &ds, CasterPlayerState &cps);

#endif // CASTERPLAYERSTATE_H
