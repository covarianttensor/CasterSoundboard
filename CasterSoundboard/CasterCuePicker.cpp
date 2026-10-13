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
#include "CasterCuePicker.h"
#include "CasterPlayer.h"
#include <QMediaPlayer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

CasterCuePicker::CasterCuePicker(int _startTime, int _stopTime, QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle("CUE Sound");
    ok = false;
    startTime = _startTime;
    stopTime = _stopTime;
    loop = false;
    player = new CasterMediaPlayer(this);

    // UI
    label_StartTimeText = new QLabel("Start\nTime");
    label_StopTimeText = new QLabel("Stop\nTime");
    label_StartTime = new QLabel("00:00");
    label_StopTime = new QLabel("00:00");
    slider_StartTime = new QSlider(Qt::Horizontal);
    slider_StartTime->setRange(0, 100);
    slider_StartTime->setValue(0);
    slider_StartTime->setStyleSheet("QSlider:horizontal {"
                                "min-height: 30px;"
                                "}"
                                "QSlider::groove:horizontal { "
                                "border: 1px solid #999999; "
                                "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #d6d6d6, stop:1 #999999); "
                                "margin: 8px 0; "
                                "} "
                                "QSlider::handle:horizontal { "
                                "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ffffff, stop:1 #565656); "
                                "border: 2px solid #000000; "
                                "width: 40px;"
                                "margin: -8px 0px; "
                                "} ");// Style the track bar (Touch Friendly)
    slider_StopTime = new QSlider(Qt::Horizontal);
    slider_StopTime->setRange(0, 100);
    slider_StopTime->setValue(100);
    slider_StopTime->setStyleSheet("QSlider:horizontal {"
                                "min-height: 30px;"
                                "}"
                                "QSlider::groove:horizontal { "
                                "border: 1px solid #999999; "
                                "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #d6d6d6, stop:1 #999999); "
                                "margin: 8px 0; "
                                "} "
                                "QSlider::handle:horizontal { "
                                "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ffffff, stop:1 #565656); "
                                "border: 2px solid #000000; "
                                "width: 40px;"
                                "margin: -8px 0px; "
                                "} ");// Style the track bar (Touch Friendly)
    button_Preview = new QPushButton("Preview");
    button_Cancel = new QPushButton("Cancel");
    button_Accept = new QPushButton("Accept");

    //Layout
    layout_main = new QVBoxLayout(this);
    layout_startTimeWidgets = new QHBoxLayout;
    layout_stopTimeWidgets = new QHBoxLayout;
    layout_buttons = new QHBoxLayout;

    // Set Layouts
    layout_startTimeWidgets->addWidget(label_StartTimeText);
    layout_startTimeWidgets->addWidget(slider_StartTime);
    layout_startTimeWidgets->addWidget(label_StartTime);
    layout_stopTimeWidgets->addWidget(label_StopTimeText);
    layout_stopTimeWidgets->addWidget(slider_StopTime);
    layout_stopTimeWidgets->addWidget(label_StopTime);
    layout_buttons->addWidget(button_Accept);
    layout_buttons->addWidget(button_Cancel);
    layout_buttons->addWidget(button_Preview);
    layout_main->addLayout(layout_startTimeWidgets);
    layout_main->addLayout(layout_stopTimeWidgets);
    layout_main->addLayout(layout_buttons);

    //Connect Sub-Widget Events
    connect(button_Preview,SIGNAL(clicked()),this,SLOT(button_Preview_Clicked()));
    connect(button_Cancel,SIGNAL(clicked()),this,SLOT(button_Cancel_Clicked()));
    connect(button_Accept,SIGNAL(clicked()),this,SLOT(button_Accept_Clicked()));
    connect(slider_StartTime,SIGNAL(valueChanged(int)),this,SLOT(slider_StartTime_valueChanged(int)));
    connect(slider_StopTime,SIGNAL(valueChanged(int)),this,SLOT(slider_StopTime_valueChanged(int)));
    connect(player,SIGNAL(positionChanged(qint64)),this,SLOT(playerPositionChanged(qint64)), Qt::QueuedConnection);//thread safe calls to player->stop()
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &CasterCuePicker::playerMediaStatusChanged, Qt::QueuedConnection);
}


void CasterCuePicker::slider_StartTime_valueChanged(int value)
{
    if(value >= slider_StopTime->value()){
        slider_StartTime->setValue(qMax(0, slider_StopTime->value() - 1));
        return;
    }

    this->startTime = (int)((value * (qint64)duration) / 100);
    this->label_StartTime->setText(this->getTimeString(this->startTime));
    player->setPosition(this->startTime);
}

void CasterCuePicker::slider_StopTime_valueChanged(int value)
{
    if(value <= slider_StartTime->value()){
        slider_StopTime->setValue(qMin(100, slider_StartTime->value() + 1));
        return;
    }

    this->stopTime = (int)((value * (qint64)duration) / 100);
    this->label_StopTime->setText(this->getTimeString(this->stopTime));
}

void CasterCuePicker::button_Preview_Clicked()
{
    //CURRENT PLAY STATE
    if(player->state() == QMediaPlayer::PlayingState)
    {
        player->stop();
        button_Preview->setText("Preview");
    }
    else
    {
        player->setPosition(this->startTime);
        player->play();
        button_Preview->setText("Stop");
    }
}

void CasterCuePicker::button_Accept_Clicked()
{
    ok = true;
    this->close();
}

void CasterCuePicker::button_Cancel_Clicked()
{
    ok = false;
    this->close();
}

//=======Protected=======
void CasterCuePicker::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    player->stop();
    player->setMedia(QUrl());
    button_Preview->setText("Preview");
}

//======Get Private Methods======


QString CasterCuePicker::getTimeString(int timeInMilliseconds)
{
    //Current Time
    int seconds = (int) (timeInMilliseconds / 1000) % 60 ;
    int minutes = (int) ((timeInMilliseconds / (1000*60)) % 60);
    QString time = QString("%1").arg(minutes, 2, 10, QChar('0')) + ":" + QString("%1").arg(seconds, 2, 10, QChar('0'));
    return time;
}

//======Set Properties======

void CasterCuePicker::setVolume(int _volume)
{
    player->setVolume(_volume);
}

void CasterCuePicker::setFilePath(QString *_filePath, int _duration, bool _loop)
{
    player->setMedia(QUrl::fromLocalFile(_filePath->toUtf8().constData()));
    duration = _duration;
    loop = _loop;
}

void CasterCuePicker::setStartTime(int _startTime)
{
    this->startTime = _startTime;
    this->label_StartTime->setText(this->getTimeString(this->startTime));
    slider_StartTime->blockSignals(true);
    slider_StartTime->setValue(duration > 0 ? (int)(startTime / (float)duration * 100) : 0);
    slider_StartTime->blockSignals(false);
}

void CasterCuePicker::setStopTime(int _stopTime)
{
    this->stopTime = _stopTime;
    this->label_StopTime->setText(this->getTimeString(this->stopTime));
    slider_StopTime->blockSignals(true);
    slider_StopTime->setValue(duration > 0 ? (int)(stopTime / (float)duration * 100) : 100);
    slider_StopTime->blockSignals(false);
}

void CasterCuePicker::playerPositionChanged(qint64 position)
{
    if (player->position() < position && player->mediaStatus() != QMediaPlayer::EndOfMedia)
        return;
    if ((player->state() != QMediaPlayer::PlayingState
            && player->mediaStatus() != QMediaPlayer::EndOfMedia)
            || stopTime <= startTime)
        return;
    const qint64 currentPosition = (player->mediaStatus() == QMediaPlayer::EndOfMedia)
            ? qMax(position, player->position())
            : player->position();
    if((int)currentPosition >= this->stopTime)
    {
        if(loop){
            player->setPosition(this->startTime);
            player->play();
        } else {
            player->stop();//make sure to use Qt::QueuedConnection
            button_Preview->setText("Preview");
        }
    }

}

void CasterCuePicker::playerMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status != QMediaPlayer::EndOfMedia
            || player->mediaStatus() != QMediaPlayer::EndOfMedia
            || stopTime <= startTime)
        return;
    if (loop) {
        player->setPosition(this->startTime);
        player->play();
    } else {
        player->stop();
        button_Preview->setText("Preview");
    }
}
