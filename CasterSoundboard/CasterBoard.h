/*
 *
 *
 * Copyright 2013 Oscar Cerna
 *
 * This file is part of CasterSoundboard. An application for playing hot-keyed sounds.
 * For more information, please visit http://sourceforge.net/projects/castersb.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LESSER GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU LESSER GENERAL PUBLIC LICENSE
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef CASTERBOARD_H
#define CASTERBOARD_H
#include <QWidget>
#include <QDataStream>
#include <QString>
#include <QMap>
#include "libs/osc/composer/OscMessageComposer.h"

//forward declarations
class CasterPlayerWidget;
class QString;
class CasterBoardState;

class CasterBoard : public QWidget //inherit from QWidget
{
    Q_OBJECT
public:
    //Constructor
    CasterBoard(QWidget* parent = 0); //don't forget to pass the parent

    //Properties
    bool isCurrentBoard = false;
    QString *soundBoardName;
    QString *profileFilePath;
    QMap<int,QString> *int_to_player_key;
    QMap<int,QString> *keyboard_key_to_player_key;
    QMap<QString, CasterPlayerWidget*> *players;

    //Methods
    void stopAllSounds();
    void reloadBoardFromPlayerStates();
    void setAllAudioDuckingStates(int state);
    void syncWithOSCClient();//Refeshes UI on OSC Client


protected:
    //========
    void keyReleaseEvent(QKeyEvent *event);//Capture Hot Keys

private:
    //Private Methods
    //OSC Composer Methods
    OscMessageComposer* writeOSCMessage(QString address, int value);
    OscMessageComposer* writeOSCMessage(QString address, float value);
    OscMessageComposer* writeOSCMessage(QString address, QString value);

signals:
    //SIGNALS
    void globalHotKeyReleasedEvent(QKeyEvent *event);//Use by mainwindow to perform actions on all sound boads. Needed because of how focus works in qt.
    void _updateOSCClient(OscMessageComposer* message);

public slots:
    //SLOTS
    void notifyApplicationAboutOSCMessage(OscMessageComposer* message);

};

//Operator Overloading
QDataStream &operator<<(QDataStream &ds, const CasterBoard &cb);
QDataStream &operator>>(QDataStream &ds, CasterBoard &cb);

#endif // CASTERBOARD_H
