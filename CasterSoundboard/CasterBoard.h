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
    QString *soundBoardName;
    QString *profileFilePath;

    //WIDGETS
    CasterPlayerWidget *player1;
    CasterPlayerWidget *player2;
    CasterPlayerWidget *player3;
    CasterPlayerWidget *player4;
    CasterPlayerWidget *player5;
    CasterPlayerWidget *player6;
    CasterPlayerWidget *player7;
    CasterPlayerWidget *player8;

    CasterPlayerWidget *playerQ;
    CasterPlayerWidget *playerW;
    CasterPlayerWidget *playerE;
    CasterPlayerWidget *playerR;
    CasterPlayerWidget *playerT;
    CasterPlayerWidget *playerY;
    CasterPlayerWidget *playerU;
    CasterPlayerWidget *playerI;

    CasterPlayerWidget *playerA;
    CasterPlayerWidget *playerS;
    CasterPlayerWidget *playerD;
    CasterPlayerWidget *playerF;
    CasterPlayerWidget *playerG;
    CasterPlayerWidget *playerH;
    CasterPlayerWidget *playerJ;
    CasterPlayerWidget *playerK;

    CasterPlayerWidget *playerZ;
    CasterPlayerWidget *playerX;
    CasterPlayerWidget *playerC;
    CasterPlayerWidget *playerV;
    CasterPlayerWidget *playerB;
    CasterPlayerWidget *playerN;
    CasterPlayerWidget *playerM;
    CasterPlayerWidget *playerCOMMA;

    //Methods
    void stopAllSounds();
    void reloadBoardFromPlayerStates();


protected:
    //========
    void keyReleaseEvent(QKeyEvent *event);//Capture Hot Keys

private:
    //Private Methods

signals:
    //SIGNALS

public slots:
    //SLOTS

};

//Operator Overloading
QDataStream &operator<<(QDataStream &ds, const CasterBoard &cb);
QDataStream &operator>>(QDataStream &ds, CasterBoard &cb);

#endif // CASTERBOARD_H
