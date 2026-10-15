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
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFile>
#include <QDataStream>
#include <QString>
#include <QScrollArea>
#include "libs/osc/composer/OscMessageComposer.h"

//Constructor=============================================
CasterBoard::CasterBoard(QWidget* parent) : QWidget(parent)
{
    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QScrollArea *playersScrollArea = new QScrollArea(this);
    QWidget *boardWidget = new QWidget(playersScrollArea);
    boardLayout = new QGridLayout();
    boardLayout->setContentsMargins(0, 0, 0, 0);
    boardWidget->setMinimumWidth(1700);
    boardWidget->setMinimumHeight(900);
    boardWidget->setLayout(boardLayout);
    playersScrollArea->setWidget(boardWidget);
    mainLayout->addWidget(playersScrollArea, 0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    //Properties
    soundBoardName = new QString("No Name");
    profileFilePath = new QString("");


    // MAPs
    // int_to_player_key
    int_to_player_key = new QMap<int,QString>
    {
        {0, "1"},{1,"2"},{2,"3"},{3,"4"},{4,"5"},{5,"6"},{6,"7"},{7,"8"},
        {8, "Q"},{9,"W"},{10,"E"},{11,"R"},{12,"T"},{13,"Y"},{14,"U"},{15,"I"},
        {16,"A"},{17,"S"},{18,"D"},{19,"F"},{20,"G"},{21,"H"},{22,"J"},{23,"K"},
        {24,"Z"},{25,"X"},{26,"C"},{27,"V"},{28,"B"},{29,"N"},{30,"M"},{31,","}
    };
    // keyboard_key_to_player_key
    keyboard_key_to_player_key = new QMap<int,QString>
    {
        {Qt::Key_1,"1"},{Qt::Key_2,"2"},{Qt::Key_3,"3"},{Qt::Key_4,"4"},{Qt::Key_5,"5"},{Qt::Key_6,"6"},{Qt::Key_7,"7"},{Qt::Key_8,"8"},
        {Qt::Key_Q, "Q"},{Qt::Key_W,"W"},{Qt::Key_E,"E"},{Qt::Key_R,"R"},{Qt::Key_T,"T"},{Qt::Key_Y,"Y"},{Qt::Key_U,"U"},{Qt::Key_I,"I"},
        {Qt::Key_A,"A"},{Qt::Key_S,"S"},{Qt::Key_D,"D"},{Qt::Key_F,"F"},{Qt::Key_G,"G"},{Qt::Key_H,"H"},{Qt::Key_J,"J"},{Qt::Key_K,"K"},
        {Qt::Key_Z,"Z"},{Qt::Key_X,"X"},{Qt::Key_C,"C"},{Qt::Key_V,"V"},{Qt::Key_B,"B"},{Qt::Key_N,"N"},{Qt::Key_M,"M"},{Qt::Key_Comma,","}
    };
    // players
    players = new QMap<QString, CasterPlayerWidget*>();

    /* Create Players */
    int _board_column = 0; int _board_row = 0;
    for(int i=0; i < int_to_player_key->count(); i++){
        // Initialize player
        players->insert(int_to_player_key->value(i), new CasterPlayerWidget(boardWidget));
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

CasterBoard::~CasterBoard()
{
    delete soundBoardName;
    delete profileFilePath;
    delete int_to_player_key;
    delete keyboard_key_to_player_key;
    delete players;
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
    else
        delete message; // No transport receiver owns messages from inactive tabs.
}

QPoint CasterBoard::playerGridPosition(const QString &key) const
{
    const int index = boardLayout->indexOf(players->value(key, nullptr));
    if (index < 0) return QPoint(-1, -1);
    int row, column, rowSpan, columnSpan;
    boardLayout->getItemPosition(index, &row, &column, &rowSpan, &columnSpan);
    return QPoint(column, row);
}

void CasterBoard::syncWithOSCClient()
{
    if (!isCurrentBoard)
        return;

    // Update Tab Name
    OscMessageComposer* msg = writeOSCMessage("/glo/m/label/tab_name", *soundBoardName);
    emit this->_updateOSCClient(msg);

    // Iterate through players
    foreach(QString _letter, players->keys()){
        CasterPlayerWidget *player = players->value(_letter);
        player->syncWithOSCClient();
        // Query the same layout that places the widgets, not the player-key order.
        const int index = boardLayout->indexOf(player);
        if (index >= 0) {
            int row, column, rowSpan, columnSpan;
            boardLayout->getItemPosition(index, &row, &column, &rowSpan, &columnSpan);
            OscMessageComposer *grid = writeOSCMessage("/cbp/" + _letter + "/m/grid", row);
            grid->pushInt32(column);
            emit _updateOSCClient(grid);
        }
    }
}

// PROTECTED
void CasterBoard::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;

    //Handles All Hot Key Behavior
    if(keyboard_key_to_player_key->contains(event->key())){
        const QString playerKey = keyboard_key_to_player_key->value(event->key());
        if(players->contains(playerKey)){
            // Play or stop sound based on player's play state
            players->value(playerKey)->play_stop_toggle();
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
    if (!cb.mainWindowSize.isEmpty())
        ds << quint32(0x4357535a) << quint32(1) << cb.mainWindowSize; // CWSZ v1
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
    cb.mainWindowSize = QSize();
    // Older profiles end after the player states.
    if (ds.status() == QDataStream::Ok && !ds.device()->atEnd()) {
        quint32 magic = 0, version = 0;
        ds >> magic >> version >> cb.mainWindowSize;
        if (magic != 0x4357535a || version != 1 || cb.mainWindowSize.isEmpty())
            ds.setStatus(QDataStream::ReadCorruptData);
    }
    return ds;
}
