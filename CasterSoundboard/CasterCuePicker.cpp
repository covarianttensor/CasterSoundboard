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
#include <QMediaPlayer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

CasterCuePicker::CasterCuePicker(int _startTime, int _stopTime)
{
    this->setWindowTitle("CUE Sound");
    ok = false;
    startTime = _startTime;
    stopTime = _stopTime;
    loop = false;
    player = new QMediaPlayer();

    // UI
    label_StartTimeText = new QLabel("Start\nTime");
    label_StopTimeText = new QLabel("Stop\nTime");
    label_StartTime = new QLabel("00:00");
    label_StopTime = new QLabel("00:00");
    slider_StartTime = new QSlider(Qt::Horizontal);
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
}


void CasterCuePicker::slider_StartTime_valueChanged(int value)
{
    if(value >= slider_StopTime->value()){
        slider_StartTime->setValue(value - 1);
    }

    this->startTime = (int)((slider_StartTime->value() * duration)/100.0);
    this->label_StartTime->setText(this->getTimeString(this->startTime));
    player->setPosition(this->startTime);
}

void CasterCuePicker::slider_StopTime_valueChanged(int value)
{
    if(value <= slider_StartTime->value()){
        slider_StopTime->setValue(value + 1);
    }

    this->stopTime = (int)((slider_StopTime->value() * duration)/100.0);
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
    this->close();
}

//=======Protected=======
void CasterCuePicker::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    player->stop();
    button_Preview->setText("Preview");
}

//======Get Private Methods======


QString CasterCuePicker::getTimeString(int timeInMilliseconds)
{
    //Current Time
    int seconds = (int) (timeInMilliseconds / 1000) % 60 ;
    int minutes = (int) ((timeInMilliseconds / (1000*60)) % 60);
    QString time = QString("%1").arg(minutes,2,'i',0,'0') + ":" + QString("%1").arg(seconds,2,'i',0,'0');
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
    slider_StartTime->setValue(startTime / (float)duration * 100);
}

void CasterCuePicker::setStopTime(int _stopTime)
{
    this->stopTime = _stopTime;
    this->label_StopTime->setText(this->getTimeString(this->stopTime));
    slider_StopTime->setValue( stopTime / (float)duration * 100);
}

void CasterCuePicker::playerPositionChanged(qint64 position)
{
    if((int)position >= this->stopTime)
    {
        if(loop){
            player->setPosition(this->startTime);
        } else {
            player->stop();//make sure to use Qt::QueuedConnection
        }
    }

}
