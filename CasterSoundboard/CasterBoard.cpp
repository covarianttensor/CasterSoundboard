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
#include "CasterBoard.h"
#include "CasterPlayer.h"
#include "CasterPlayerState.h"
#include <QGridLayout>
#include <QFile>
#include <QDataStream>
#include <QString>
#include "libs/osc/composer/OscMessageComposer.h"

//Constructor=============================================
CasterBoard::CasterBoard(QWidget* parent) : QWidget(parent)
{
    QGridLayout *boardLayout = new QGridLayout(this);

    //Properties
    soundBoardName = new QString("No Name");
    profileFilePath = new QString("");

    // MAPs
    // int_to_player_key
    int_to_player_key = new QMap<int,QString>();
    int_to_player_key->insert(0, "1"); int_to_player_key->insert(1, "2"); int_to_player_key->insert(2, "3"); int_to_player_key->insert(3, "4"); int_to_player_key->insert(4, "5"); int_to_player_key->insert(5, "6"); int_to_player_key->insert(6, "7"); int_to_player_key->insert(7, "8");
    int_to_player_key->insert(8, "Q"); int_to_player_key->insert(9, "W"); int_to_player_key->insert(10, "E"); int_to_player_key->insert(11, "R"); int_to_player_key->insert(12, "T"); int_to_player_key->insert(13, "Y"); int_to_player_key->insert(14, "U"); int_to_player_key->insert(15, "I");
    int_to_player_key->insert(16, "A"); int_to_player_key->insert(17, "S"); int_to_player_key->insert(18, "D"); int_to_player_key->insert(19, "F"); int_to_player_key->insert(20, "G"); int_to_player_key->insert(21, "H"); int_to_player_key->insert(22, "J"); int_to_player_key->insert(23, "K");
    int_to_player_key->insert(24, "Z"); int_to_player_key->insert(25, "X"); int_to_player_key->insert(26, "C"); int_to_player_key->insert(27, "V"); int_to_player_key->insert(28, "B"); int_to_player_key->insert(29, "N"); int_to_player_key->insert(30, "M"); int_to_player_key->insert(31, ",");
    // keyboard_key_to_player_key
    keyboard_key_to_player_key = new QMap<int,QString>();
    keyboard_key_to_player_key->insert(Qt::Key_1, "1"); keyboard_key_to_player_key->insert(Qt::Key_2, "2"); keyboard_key_to_player_key->insert(Qt::Key_3, "3"); keyboard_key_to_player_key->insert(Qt::Key_4, "4"); keyboard_key_to_player_key->insert(Qt::Key_5, "5"); keyboard_key_to_player_key->insert(Qt::Key_6, "6"); keyboard_key_to_player_key->insert(Qt::Key_7, "7"); keyboard_key_to_player_key->insert(Qt::Key_8, "8");
    keyboard_key_to_player_key->insert(Qt::Key_Q, "Q"); keyboard_key_to_player_key->insert(Qt::Key_W, "W"); keyboard_key_to_player_key->insert(Qt::Key_E, "E"); keyboard_key_to_player_key->insert(Qt::Key_R, "R"); keyboard_key_to_player_key->insert(Qt::Key_T, "T"); keyboard_key_to_player_key->insert(Qt::Key_Y, "Y"); keyboard_key_to_player_key->insert(Qt::Key_U, "U"); keyboard_key_to_player_key->insert(Qt::Key_I, "I");
    keyboard_key_to_player_key->insert(Qt::Key_A, "A"); keyboard_key_to_player_key->insert(Qt::Key_S, "S"); keyboard_key_to_player_key->insert(Qt::Key_D, "D"); keyboard_key_to_player_key->insert(Qt::Key_F, "F"); keyboard_key_to_player_key->insert(Qt::Key_G, "G"); keyboard_key_to_player_key->insert(Qt::Key_H, "H"); keyboard_key_to_player_key->insert(Qt::Key_J, "J"); keyboard_key_to_player_key->insert(Qt::Key_K, "K");
    keyboard_key_to_player_key->insert(Qt::Key_Z, "Z"); keyboard_key_to_player_key->insert(Qt::Key_X, "X"); keyboard_key_to_player_key->insert(Qt::Key_C, "C"); keyboard_key_to_player_key->insert(Qt::Key_V, "V"); keyboard_key_to_player_key->insert(Qt::Key_B, "B"); keyboard_key_to_player_key->insert(Qt::Key_N, "N"); keyboard_key_to_player_key->insert(Qt::Key_M, "M"); keyboard_key_to_player_key->insert(Qt::Key_Comma, ",");
    // players
    players = new QMap<QString, CasterPlayerWidget*>();

    /* Create Players */
    int _board_column = 0; int _board_row = 0;
    for(int i=0; i < int_to_player_key->count(); i++){
        // Initialize player
        players->insert(int_to_player_key->value(i), new CasterPlayerWidget);
        players->value(int_to_player_key->value(i))->setHotKeyLetter(int_to_player_key->value(i));

        // Connect OSC Events
        connect(players->value(int_to_player_key->value(i)),SIGNAL(updateOSCClient(OscMessageComposer*)),this,SLOT(notifyApplicationAboutOSCMessage(OscMessageComposer*)));

        // Add to layout
        boardLayout->addWidget(players->value(int_to_player_key->value(i)), _board_row, _board_column);

        // Update Next Layout
        _board_column += 1;
        if(_board_column > 7){
            _board_row += 1;
            _board_column = 0;
        }
    }
}
//PUBLIC
void CasterBoard::stopAllSounds()
{
    // Iterate through players
    foreach(QString _letter, players->keys()){
        players->value(_letter)->stopSound();
    }
}

void CasterBoard::setAllAudioDuckingStates(int state)
{
    // Iterate through players
    foreach(QString _letter, players->keys()){
        players->value(_letter)->setAudioDuckState(state);
    }
}

void CasterBoard::notifyApplicationAboutOSCMessage(OscMessageComposer* message){
    if(this->isCurrentBoard)
        emit this->_updateOSCClient(message);
}

void CasterBoard::syncWithOSCClient()
{
    // Update Tab Name
    OscMessageComposer* msg = writeOSCMessage("/glo/m/label/tab_name", *soundBoardName);
    emit this->_updateOSCClient(msg);

    // Iterate through players
    foreach(QString _letter, players->keys()){
        players->value(_letter)->syncWithOSCClient();
    }
}

// PROTECTED
void CasterBoard::keyReleaseEvent(QKeyEvent *event)
{
    //Handles All Hot Key Behavior
    if(keyboard_key_to_player_key->contains(event->key())){
        if(players->contains(keyboard_key_to_player_key->value(event->key()))){
            // Play or stop sound based on player's play state
            if(players->value(keyboard_key_to_player_key->value(event->key()))->player->state() == QMediaPlayer::PlayingState)
                players->value(keyboard_key_to_player_key->value(event->key()))->stopSound();
            else if (players->value(keyboard_key_to_player_key->value(event->key()))->player->state() == QMediaPlayer::PausedState)
                players->value(keyboard_key_to_player_key->value(event->key()))->playSound();
            else if (players->value(keyboard_key_to_player_key->value(event->key()))->player->state() == QMediaPlayer::StoppedState)
                players->value(keyboard_key_to_player_key->value(event->key()))->playSound();
        }
    } else {
        // Delegates global hotkeys to MainWIndow
        emit globalHotKeyReleasedEvent(event);
    }

}

//Public Methods
void CasterBoard::reloadBoardFromPlayerStates()
{
    // Iterate through players
    foreach(QString _letter, players->keys()){
        players->value(_letter)->reloadFromPlayerState();
    }

    this->update();
}

//========================================================
//==========OSC Composer Methods=====
OscMessageComposer* CasterBoard::writeOSCMessage(QString address, int value){
    // Compose OSC Message
    OscMessageComposer* msg = new OscMessageComposer(address);
    msg->pushInt32((qint32)value);
    return msg;
}

OscMessageComposer* CasterBoard::writeOSCMessage(QString address, float value){
    // Compose OSC Message
    OscMessageComposer* msg = new OscMessageComposer(address);
    msg->pushFloat(value);
    return msg;
}

OscMessageComposer* CasterBoard::writeOSCMessage(QString address, QString value){
    // Compose OSC Message
    OscMessageComposer* msg = new OscMessageComposer(address);
    msg->pushString(value.toUtf8());
    return msg;
}

//================================================


//Operator Overloading
QDataStream &operator<<(QDataStream &ds, const CasterBoard &cb)
{
    /* Properties */
    ds << *cb.soundBoardName;
    /* All Player Properties */
    // Iterate through players
    foreach(QString _letter, cb.players->keys()){
        ds << *cb.players->value(_letter)->playerState;
    }
    return ds;
}


QDataStream &operator>>(QDataStream &ds, CasterBoard &cb)
{
    /* Properties */
    ds >> *cb.soundBoardName;
    /* All Player Properties */
    // Iterate through players
    foreach(QString _letter, cb.players->keys()){
        ds >> *cb.players->value(_letter)->playerState;
    }
    return ds;
}
