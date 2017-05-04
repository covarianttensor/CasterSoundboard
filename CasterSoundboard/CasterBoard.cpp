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
    Qt::Key const keys[] = {
        Qt::Key_1, Qt::Key_2, Qt::Key_3, Qt::Key_4,
        Qt::Key_5, Qt::Key_6, Qt::Key_7, Qt::Key_8,

        Qt::Key_Q, Qt::Key_W, Qt::Key_E, Qt::Key_R,
        Qt::Key_T, Qt::Key_Y, Qt::Key_U, Qt::Key_I,

        Qt::Key_A, Qt::Key_S, Qt::Key_D, Qt::Key_F,
        Qt::Key_G, Qt::Key_H, Qt::Key_J, Qt::Key_K,

        Qt::Key_Z, Qt::Key_X, Qt::Key_C, Qt::Key_V,
        Qt::Key_B, Qt::Key_N, Qt::Key_M, Qt::Key_Comma,
    };

    QString const letters =
        "12345678"
        "QWERTYUI"
        "ASDFGHJK"
        "ZXCVBNM,";

    int offset = 0;
    for (QChar const letter : letters) {
        int posX = offset / 8;
        int posY = offset % 8;
        layout[keys[offset++]] = new CasterBoardLayout(letter, posX, posY);
    }

    QGridLayout *boardLayout = new QGridLayout(this);

    for (int key : layout.keys()) {
        CasterBoardLayout *value = layout.value(key);
        CasterPlayerWidget *widget = new CasterPlayerWidget();
        value->widget = widget;
        widget->setHotKeyLetter(value->letter);
        boardLayout->addWidget(widget, value->posY, value->posX);
    }
}

void CasterBoard::saveLayout(QSettings &settings)
{
    for (int key : layout.keys()) {
        CasterBoardLayout *value = layout.value(key);
        CasterPlayerWidget *widget = value->widget;
        QString keystr = QString::number(key);
        if (!widget->soundFilePath->isEmpty())
            settings.setValue(keystr, *widget->soundFilePath);
    }
}

void CasterBoard::restoreLayout(const QSettings &settings)
{
    for (int key : layout.keys()) {
        CasterBoardLayout *value = layout.value(key);
        CasterPlayerWidget *widget = value->widget;
        QString keystr = QString::number(key);
        if (settings.contains(keystr))
            widget->assignFile(settings.value(keystr).toString());
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
