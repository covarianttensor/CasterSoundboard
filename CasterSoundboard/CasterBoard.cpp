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
#include <QGridLayout>

//Constructor=============================================
CasterBoard::CasterBoard(QWidget* parent) : QWidget(parent)
{
    layout[Qt::Key_1]     = new CasterBoardLayout("1", 0, 0);
    layout[Qt::Key_2]     = new CasterBoardLayout("2", 0, 1);
    layout[Qt::Key_3]     = new CasterBoardLayout("3", 0, 2);
    layout[Qt::Key_4]     = new CasterBoardLayout("4", 0, 3);
    layout[Qt::Key_5]     = new CasterBoardLayout("5", 0, 4);
    layout[Qt::Key_6]     = new CasterBoardLayout("6", 0, 5);
    layout[Qt::Key_7]     = new CasterBoardLayout("7", 0, 6);
    layout[Qt::Key_8]     = new CasterBoardLayout("8", 0, 7);

    layout[Qt::Key_Q]     = new CasterBoardLayout("Q", 1, 0);
    layout[Qt::Key_W]     = new CasterBoardLayout("W", 1, 1);
    layout[Qt::Key_E]     = new CasterBoardLayout("E", 1, 2);
    layout[Qt::Key_R]     = new CasterBoardLayout("R", 1, 3);
    layout[Qt::Key_T]     = new CasterBoardLayout("T", 1, 4);
    layout[Qt::Key_Y]     = new CasterBoardLayout("Y", 1, 5);
    layout[Qt::Key_U]     = new CasterBoardLayout("U", 1, 6);
    layout[Qt::Key_I]     = new CasterBoardLayout("I", 1, 7);

    layout[Qt::Key_A]     = new CasterBoardLayout("A", 2, 0);
    layout[Qt::Key_S]     = new CasterBoardLayout("S", 2, 1);
    layout[Qt::Key_D]     = new CasterBoardLayout("D", 2, 2);
    layout[Qt::Key_F]     = new CasterBoardLayout("F", 2, 3);
    layout[Qt::Key_G]     = new CasterBoardLayout("G", 2, 4);
    layout[Qt::Key_H]     = new CasterBoardLayout("H", 2, 5);
    layout[Qt::Key_J]     = new CasterBoardLayout("J", 2, 6);
    layout[Qt::Key_K]     = new CasterBoardLayout("K", 2, 7);

    layout[Qt::Key_Z]     = new CasterBoardLayout("Z", 3, 0);
    layout[Qt::Key_X]     = new CasterBoardLayout("X", 3, 1);
    layout[Qt::Key_C]     = new CasterBoardLayout("C", 3, 2);
    layout[Qt::Key_V]     = new CasterBoardLayout("V", 3, 3);
    layout[Qt::Key_B]     = new CasterBoardLayout("B", 3, 4);
    layout[Qt::Key_N]     = new CasterBoardLayout("N", 3, 5);
    layout[Qt::Key_M]     = new CasterBoardLayout("M", 3, 6);
    layout[Qt::Key_Comma] = new CasterBoardLayout(",", 3, 7);

    QGridLayout *boardLayout = new QGridLayout(this);

    QHash<int, CasterBoardLayout*>::const_iterator i = layout.constBegin();

    while (i != layout.constEnd()) {
        CasterPlayerWidget *widget = new CasterPlayerWidget();
        i.value()->widget = widget;
        widget->setHotKeyLetter(i.value()->letter);
        boardLayout->addWidget(widget, i.value()->posY, i.value()->posX);
        ++i;
    }
}

void CasterBoard::keyReleaseEvent(QKeyEvent *event)
{
    //Handles All Hot Key Behavior
    if (!layout.contains(event->key()))
        return;

    CasterPlayerWidget *widget = layout.value(event->key())->widget;

    switch (widget->player->state()) {
        case QMediaPlayer::PlayingState: widget->stopSound(); break;
        case QMediaPlayer::PausedState:  widget->playSound(); break;
        case QMediaPlayer::StoppedState: widget->playSound(); break;
    }
}
