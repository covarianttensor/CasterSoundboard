
#include "CasterPlayerState.h"
#include <QString>

//Constructor
CasterPlayerState::CasterPlayerState()
{
    //Set Default Property Values
    filePath = new QString("");
    volume = 100;
    startTime = 0;
    stopTime = 0;
    loop = false;
    timeSet = false;
    // Set Slider Defaults
    slider_H_Value = 0;
    slider_S_Value = 0;
    slider_L_Value = 0;
    PlayerBackgroundCSS = new QString("");
    backgroundCSSChanged = false;
}

//Operator Overloading
QDataStream &operator<<(QDataStream &ds, const CasterPlayerState &cps)
{
    return ds << *cps.filePath << cps.volume
              << cps.startTime << cps.stopTime
              << cps.loop << cps.timeSet
              << cps.slider_H_Value << cps.slider_S_Value
              << cps.slider_L_Value << *cps.PlayerBackgroundCSS
              << cps.backgroundCSSChanged;
}

QDataStream &operator>>(QDataStream &ds, CasterPlayerState &cps)
{
    return ds >> *cps.filePath >> cps.volume
              >> cps.startTime >> cps.stopTime
              >> cps.loop >> cps.timeSet
              >> cps.slider_H_Value >> cps.slider_S_Value
              >> cps.slider_L_Value >> *cps.PlayerBackgroundCSS
              >> cps.backgroundCSSChanged;
}
