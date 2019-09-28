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
#include "CasterPlayer.h"
#include "CasterLabelColorPicker.h"
#include "CasterCuePicker.h"
#include "CasterPlayerState.h"
#include "CSS.h"
#include <QSizePolicy>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
//#include <QtMultimedia/QMediaPlayer>
#include <QMediaMetaData>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QWidget>
#include <QStyleOption>
#include <QPainter>
#include <QImage>
#include <QPainterPath>
#include <QBrush>
#include <QLinearGradient>
#include <QGraphicsDropShadowEffect>
#include <QFileDialog>
#include "libs/osc/composer/OscMessageComposer.h"

//Constructor
CasterPlayerWidget::CasterPlayerWidget(QWidget* parent) : QWidget(parent)
{
    // ID
    id = new QString("");

    // UI Settings
    this->setMinimumWidth(200);
    this->setMinimumHeight(200);
    this->setMaximumWidth(200);
    this->setMaximumHeight(200);

    //Set Widget Defaults
    this->setAcceptDrops(true);

    //Init Player
    player = new QMediaPlayer(this);
    playStateImage = new QImage;
    playStateImage->load(":/res/img/playState_playing.png");
    //Init Properties
    playerState = new CasterPlayerState;
    soundFilePath = new QString("");
    hotKeyLetter = new QString("1");
    progress = 0.0;
    playerState->volume = 100;
    trackBarWasChangedByPlayer = false;

    isAudioDucked = false;

    //Diag
    cuePicker = new CasterCuePicker(0, 0);
    colorPicker = new CasterLabelColorPicker();

    //Internal Properties
    newMediaLoaded = false;
    newMediaLoadedFromProfile = false;

    //Set-Up Widget Layout
    soundNameLabel = new QLabel("<Drop File>");
    soundNameLabel->setStyleSheet("background-color:transparent;color:black;");
    soundNameLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    QFont sNLF("Georgia",10,-1,false); sNLF.setBold(true);
    soundNameLabel->setFont(sNLF);
    soundNameLabel->setWordWrap(true);
    soundNameLabel->setMaximumWidth(150);
    //soundNameLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    hotKeyLabel = new QLabel(*hotKeyLetter);
    hotKeyLabel->setAlignment(Qt::AlignCenter);
    hotKeyLabel->setFont(QFont("Georgia",13,-1,false));
    hotKeyLabel->setStyleSheet("background:url(:/res/img/Key.png) no-repeat;background-attachment:fixed;background-position:center;color:white;");
    //hotKeyLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    timeLabel = new QLabel("+00:00\n-00:00");
    timeLabel->setStyleSheet("background-color:transparent;color:black;");
    timeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignRight);
    QFont tLF("Georgia",9,-1,false); tLF.setBold(true);
    timeLabel->setFont(tLF);
    /* Play Button */
    playStateButton = new QPushButton("");
    playStateButton->setIcon(QIcon(":/res/img/stop.png"));
    playStateButton->setIconSize(QSize(18,18));
    playStateButton->setFlat(true);
    //playStateButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    /* Open File Button */
    openFileButton = new QPushButton("");
    openFileButton->setIcon(QIcon(":/res/img/openMusic.png"));
    openFileButton->setIconSize(QSize(28,28));
    openFileButton->setFlat(true);
    /* Set Cue Button */
    setCueButton = new QPushButton("");
    setCueButton->setIcon(QIcon(":/res/img/cue.png"));
    setCueButton->setIconSize(QSize(28,28));
    setCueButton->setFlat(true);
    /* Toggle Loop Button */
    toggleLoopButton = new QPushButton("");
    toggleLoopButton->setIcon(QIcon(":/res/img/no_loop"));
    toggleLoopButton->setIconSize(QSize(28,28));
    toggleLoopButton->setFlat(true);
    /* Sub Menu Button */
    colorPickerButton = new QPushButton("");
    colorPickerButton->setIcon(QIcon(":/res/img/colorPicker.png"));
    colorPickerButton->setIconSize(QSize(16,16));
    colorPickerButton->setFlat(true);
    //colorPickerButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    /* Edit Notes Button */
    editNotesButton = new QPushButton("");
    editNotesButton->setIcon(QIcon(":/res/img/notes.png"));
    editNotesButton->setIconSize(QSize(28,28));
    editNotesButton->setFlat(true);
    /* Volume Slider */
    volumeSlider = new QSlider(Qt::Vertical);
    volumeSlider->setValue(100);
    volumeSlider->setStyleSheet("QSlider:vertical {"
                                "min-width: 30px;"
                                "background-color:transparent;"
                                "}"
                                "QSlider::groove:vertical { "
                                "border: 1px solid #999999; "
                                "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #d6d6d6, stop:1 #999999); "
                                "margin: 8px 0; "
                                "} "
                                "QSlider::handle:vertical { "
                                "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ffffff, stop:1 #565656); "
                                "border: 2px solid #000000; "
                                "height: 40px;"
                                "margin: -8px 0px; "
                                "} ");// Style the volume slider (Touch Friendly)
    /* Track Bar */
    trackBar = new QSlider(Qt::Horizontal);
    trackBar->setValue(0);
    trackBar->setStyleSheet("QSlider:horizontal {"
                                "min-height: 30px;"
                                "background-color:transparent;"
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

    //Place Widgets
    mainLayout = new QVBoxLayout(this);
    //mainLayout->setContentsMargins(0,0,0,0);

    subMainLayoutH = new QHBoxLayout;
    subMainLayoutV = new QVBoxLayout;

    topLayout = new QHBoxLayout;
    topLayout->addWidget(hotKeyLabel);
    topLayout->addWidget(timeLabel);

    centerLayout = new QHBoxLayout;
    centerLayout->addWidget(soundNameLabel);

    /* Layout: Bottom Player Buttons */
    bottomLayout_TopButtons = new QHBoxLayout;
    //bottomLayout_TopButtons->addWidget(playStateButton);
    bottomLayout_TopButtons->addWidget(openFileButton);
    bottomLayout_TopButtons->addWidget(setCueButton);
    bottomLayout_TopButtons->addWidget(toggleLoopButton);

    bottomLayout_BottomButtons = new QHBoxLayout;
    bottomLayout_BottomButtons->addWidget(colorPickerButton);
    //bottomLayout_BottomButtons->addWidget(editNotesButton);

    bottomLayout = new QVBoxLayout;
    bottomLayout->addLayout(bottomLayout_TopButtons);
    bottomLayout->addLayout(bottomLayout_BottomButtons);


    subMainLayoutV->addLayout(topLayout);
    subMainLayoutV->addLayout(centerLayout);
    subMainLayoutV->addLayout(bottomLayout);

    subMainLayoutH->addLayout(subMainLayoutV);
    subMainLayoutH->addWidget(volumeSlider);

    mainLayout->addLayout(subMainLayoutH);
    mainLayout->addWidget(trackBar);

    //Widget Styling
    this->setMouseTracking(true);
    this->setStyleSheet(CSS_GRAIENT_GREY);
    QGraphicsDropShadowEffect * dse = new QGraphicsDropShadowEffect();
    dse->setBlurRadius(10);
    this->setGraphicsEffect(dse);

    //Connect Sub-Widget Events
    connect(playStateButton,SIGNAL(clicked()),this,SLOT(playerToggle()));
    connect(volumeSlider,SIGNAL(valueChanged(int)),this,SLOT(volumeChanged(int)));
    connect(trackBar,SIGNAL(valueChanged(int)),this,SLOT(trackBarChanged(int)));
    connect(openFileButton,SIGNAL(clicked()),this,SLOT(openFileDiag()));
    connect(colorPickerButton,SIGNAL(clicked()),this,SLOT(openColorPicker()));
    connect(setCueButton,SIGNAL(clicked()),this,SLOT(openCuePicker()));
    connect(toggleLoopButton,SIGNAL(clicked()),this,SLOT(toggleLooping()));
    connect(player,SIGNAL(positionChanged(qint64)),this,SLOT(playerPositionChanged(qint64)),Qt::QueuedConnection);
    connect(player,SIGNAL(stateChanged(QMediaPlayer::State)),this,SLOT(playerStateChanged(QMediaPlayer::State)));
    connect(player,SIGNAL(metaDataChanged()),this,SLOT(playerMetaDataChanged()));
    connect(player,SIGNAL(durationChanged(qint64)),this,SLOT(playerDurationChanged(qint64)));
}

//Set Properties
void CasterPlayerWidget::setHotKeyLetter(QString hotKey)
{
    hotKeyLabel->setText(hotKey);
    this->id = new QString(hotKey.toUtf8());
}

void CasterPlayerWidget::syncWithOSCClient()
{
    //Update Volume
    OscMessageComposer* msg1 = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/vol", (float)(volumeSlider->value()/100.0) );
    emit updateOSCClient(msg1);

    //Update Track Position
    OscMessageComposer* msg2 = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/t_p_p", (float)((float)trackBar->value()/100.0) );
    emit updateOSCClient(msg2);

    //Update Title
    OscMessageComposer* msg3 = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/tr_name", this->soundNameLabel->text());
    emit updateOSCClient(msg3);

    //Update Loop State
    OscMessageComposer* msg4 = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/l_s", (int)(playerState->loop));
    emit updateOSCClient(msg4);

    //Update Time
    QStringList times = this->timeLabel->text().split("\n");
    OscMessageComposer* msg5 = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/time", times.value(0) + times.value(1));
    emit updateOSCClient(msg5);

    //Update Play State
    if(player->state() == QMediaPlayer::PlayingState){
        OscMessageComposer* msg6 = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/p_s", "Playing");
        emit updateOSCClient(msg6);
    }
    else if(player->state() == QMediaPlayer::PausedState){
        OscMessageComposer* msg6 = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/p_s", "Paused");
        emit updateOSCClient(msg6);
    }
    else if (player->state() == QMediaPlayer::StoppedState){
        OscMessageComposer* msg6 = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/p_s", "Stopped");
        emit updateOSCClient(msg6);
    }

}

//----SIGNALS----

//-----SLOTS----
void CasterPlayerWidget::playerToggle()
{
    //CURRENT PLAY STATE TOGGLE LOGIC
    volumeSlider->setValue(playerState->volume);
    if(player->state() == QMediaPlayer::PlayingState)
    {
        player->stop();
    }
    else if (player->state() == QMediaPlayer::StoppedState)
    {
        player->setPosition(playerState->startTime);
        player->play();
    }
    else if(player->state() == QMediaPlayer::PausedState)
    {
        player->setPosition(playerState->startTime);
        player->play();
    }
}

void CasterPlayerWidget::volumeChanged(int value)
{
    //Update Player Save State
    playerState->volume = value;
    if(isAudioDucked)
        player->setVolume(playerState->volume * 0.33);
    else
        player->setVolume(playerState->volume);

    //Update OSC Client
    OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/vol", (float)(playerState->volume/100.0) );
    emit updateOSCClient(msg);
}

/* Track Barn Position Changed */
void CasterPlayerWidget::trackBarChanged(int value)
{
    if(!trackBarWasChangedByPlayer)
    {
        qint64 position = (qint64)((float)(value) * (float)(player->duration()))/100.0;
        player->setPosition(position);
    }

    //Update OSC Client
    OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/t_p_p", (float)((float)trackBar->value()/100.0) );
    emit updateOSCClient(msg);

}

/* Load Sound From Dialog */
void CasterPlayerWidget::openFileDiag()
{
    QString _filePath = QFileDialog::getOpenFileName(
            this, "Open audio file", "",
            "Audio files (*.mp3 *.wav *.ogg *.flac *.m4a);;"
            "Video files (*.mp4 *.mov *.ogv *.avi *.mpg *.wmv)");

    if (!_filePath.isNull())
    {
        QStringList pathList;
        pathList.append(_filePath);

        if(!openFiles(pathList))
        {
            QMessageBox msgBox;
            msgBox.setText("CasterSoundboard does not accept this file type.\nMake sure your system has the necessary codecs installed.\nCasterSoundboard can also play the audio from videos.");
            msgBox.setInformativeText("Try: (*.mp3), (*.wav), (*.ogg), (*.flac), (*.m4a).\nAnd Try: (*.mp4), (*.mov), (*.ogv), (*.avi), (*.mpg), (*.wmv).");
            msgBox.setStandardButtons(QMessageBox::Close);
            msgBox.setDefaultButton(QMessageBox::Close);
            msgBox.setModal(true);
            msgBox.exec();
        }
    }
}

void CasterPlayerWidget::openColorPicker()
{
    //OPEN SUB MENU
    if(playerState->backgroundCSSChanged){
        colorPicker->set_slider_H_Value(playerState->slider_H_Value);
        colorPicker->set_slider_S_Value(playerState->slider_S_Value);
        colorPicker->set_slider_L_Value(playerState->slider_L_Value);
    }

    colorPicker->exec();
    if(colorPicker->ok)
    {
        this->setStyleSheet(colorPicker->currentCSS);
        // Update Player State
        playerState->PlayerBackgroundCSS = &colorPicker->currentCSS;
        playerState->backgroundCSSChanged = true;
        playerState->slider_H_Value = colorPicker->slider_H_Value;
        playerState->slider_S_Value = colorPicker->slider_S_Value;
        playerState->slider_L_Value = colorPicker->slider_L_Value;

    }
}

void CasterPlayerWidget::openCuePicker()
{
    cuePicker->setFilePath(playerState->filePath, (int)player->duration(), playerState->loop);
    cuePicker->setVolume(playerState->volume);
    if(playerState->timeSet){
        cuePicker->setStartTime(playerState->startTime);
        cuePicker->setStopTime(playerState->stopTime);
    } else {
        cuePicker->setStartTime(playerState->startTime);
        cuePicker->setStopTime((int)player->duration());
    }

    cuePicker->exec();
    if(cuePicker->ok == true)
    {
        //Accept Changes
        playerState->startTime = cuePicker->startTime;
        playerState->stopTime = cuePicker->stopTime;
        playerState->timeSet = true;
        player->setPosition(playerState->startTime);
    }
}

/* Toggles Looping */
void CasterPlayerWidget::toggleLooping()
{
    if(playerState->loop == true){
        toggleLoopButton->setIcon(QIcon(":/res/img/no_loop"));
        playerState->loop = false;

        //Update OSC Client
        OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/l_s", 0);
        emit updateOSCClient(msg);
    } else {
        toggleLoopButton->setIcon(QIcon(":/res/img/loop"));
        playerState->loop = true;

        //Update OSC Client
        OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/l_s", 1);
        emit updateOSCClient(msg);
    }
}

void CasterPlayerWidget::playerPositionChanged(qint64 position)
{
    if(player->duration() > 0)
    {
        //Player State Update
        if(!playerState->timeSet && playerState->stopTime == 0){
            playerState->stopTime = player->duration();
        }

        //Current Time
        int currentTime_seconds = (int) (position / 1000) % 60 ;
        int currentTime_minutes = (int) ((position / (1000*60)) % 60);
        QString currentTime = "+" + QString("%1").arg(currentTime_minutes,2,'i',0,'0') + ":" + QString("%1").arg(currentTime_seconds,2,'i',0,'0');
        //Time Remaining
        progress = (float)(position) / (float)(player->duration());
        int timeLeft = player->duration() - position;
        int timeLeft_seconds = (int) (timeLeft / 1000) % 60 ;
        int timeLeft_minutes = (int) ((timeLeft / (1000*60)) % 60);
        //int hours   = (int) ((timeLeft / (1000*60*60)) % 24);
        QString timeRemaining = "-" + QString("%1").arg(timeLeft_minutes,2,'i',0,'0') + ":" + QString("%1").arg(timeLeft_seconds,2,'i',0,'0');
        timeLabel->setText(currentTime + "\n" + timeRemaining);// Set Time Remaining Label
        trackBarWasChangedByPlayer = true;
        trackBar->setValue((int)(progress * 100));
        trackBarWasChangedByPlayer = false;
        this->update();

        //Update OSC Client
        OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/time", currentTime + "/" + timeRemaining);
        emit updateOSCClient(msg);

        /* Enforce Start Time And Stop And Looping */
        if(position >= playerState->stopTime)
        {
            if(playerState->loop){
                player->setPosition(playerState->startTime);
                player->play();
            } else {
                player->stop();//make sure to use Qt::QueuedConnection
                player->setPosition(playerState->startTime);
            }
        }
        if(position < playerState->startTime)
        {
            player->setPosition(playerState->startTime);
        }

    }

}

void CasterPlayerWidget::playerStateChanged(QMediaPlayer::State state)
{
    //CURRENT PLAY STATE
    if(state == QMediaPlayer::PlayingState)
    {
        playStateButton->setIcon(QIcon(":/res/img/play.png"));

        //Update OSC Client
        OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/p_s", "Playing");
        emit updateOSCClient(msg);
    }
    else if(state == QMediaPlayer::PausedState)
    {
        playStateButton->setIcon(QIcon(":/res/img/pause.png"));

        //Update OSC Client
        OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/p_s", "Paused");
        emit updateOSCClient(msg);
    }
    else if (state == QMediaPlayer::StoppedState)
    {
        playStateButton->setIcon(QIcon(":/res/img/stop.png"));

        //Update OSC Client
        OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/p_s", "Stopped");
        emit updateOSCClient(msg);

        progress = 0;
        int timeLeft = player->duration();
        int seconds = (int) (timeLeft / 1000) % 60 ;
        int minutes = (int) ((timeLeft / (1000*60)) % 60);
        QString timeRemaining = "+00:00\n-" + QString("%1").arg(minutes,2,'i',0,'0') + ":" + QString("%1").arg(seconds,2,'i',0,'0');
        timeLabel->setText(timeRemaining);
        this->update();

    }

}

void CasterPlayerWidget::playerMetaDataChanged()
{
    //Update Meta Data
    QFileInfo fi(*soundFilePath);
    if(player->metaData(QMediaMetaData::Title).toString() != "")
    {
        //Use metadata title
        soundNameLabel->setText(player->metaData(QMediaMetaData::Title).toString());

        //Update OSC Client
        OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/tr_name", player->metaData(QMediaMetaData::Title).toString());
        emit updateOSCClient(msg);
    }
    else
    {
        //Use filename as title
        soundNameLabel->setText(fi.baseName());

        //Update OSC Client
        OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/tr_name", fi.baseName());
        emit updateOSCClient(msg);
    }

    //Hack solution to prevent playing when when meadia loaded.
    if(newMediaLoaded)
    {
        newMediaLoaded = false;
        player->stop();
    }

}

void CasterPlayerWidget::playerDurationChanged(qint64 _duration)
{
    if(newMediaLoaded && !newMediaLoadedFromProfile){
        //reset player state
        playerState->startTime = 0;
        playerState->stopTime = _duration;
        playerState->timeSet = false;
        player->setPosition(playerState->startTime);
    } else if (newMediaLoadedFromProfile){
        cuePicker->setFilePath(playerState->filePath, (int)player->duration(), playerState->loop);
        cuePicker->setVolume(playerState->volume);
        if(playerState->timeSet){
            cuePicker->setStartTime(playerState->startTime);
            cuePicker->setStopTime(playerState->stopTime);
        } else {
            cuePicker->setStartTime(playerState->startTime);
            cuePicker->setStopTime((int)player->duration());
        }
    }
}

//--------------

//Private Methods
int CasterPlayerWidget::getProgressWidth()
{
    //Compute Progress width
    return (int)(this->progress * (float)(this->width()));
}

//Public Methods
//===============Player Methods=================
void CasterPlayerWidget::playSound()
{
    volumeSlider->setValue(playerState->volume);
    player->setPosition(playerState->startTime);
    player->play();
}

void CasterPlayerWidget::resumeSound()
{
    volumeSlider->setValue(playerState->volume);
    player->play();
}

void CasterPlayerWidget::pauseSound()
{
    volumeSlider->setValue(playerState->volume);
    player->pause();
}

void CasterPlayerWidget::stopSound()
{
    volumeSlider->setValue(playerState->volume);
    player->stop();
}

void CasterPlayerWidget::play_stop_toggle()
{
    // Toggle play state
    // Play/Pause
    //CURRENT PLAY STATE TOGGLE LOGIC
    volumeSlider->setValue(playerState->volume);
    if(player->state() == QMediaPlayer::PlayingState)
    {
        player->stop();
    }
    else if (player->state() == QMediaPlayer::StoppedState)
    {
        player->setPosition(playerState->startTime);
        player->play();
    }
    else if(player->state() == QMediaPlayer::PausedState)
    {
        player->setPosition(playerState->startTime);
        player->play();
    }
}

void CasterPlayerWidget::resume_pause_toggle()
{
    // Toggle play state
    // Play/Pause
    //CURRENT PLAY STATE TOGGLE LOGIC
    volumeSlider->setValue(playerState->volume);
    if(player->state() == QMediaPlayer::PlayingState)
    {
        player->pause();
    }
    else if (player->state() == QMediaPlayer::StoppedState)
    {
        player->setPosition(playerState->startTime);
        player->play();
    }
    else if(player->state() == QMediaPlayer::PausedState)
    {
        player->play();
    }
}

void CasterPlayerWidget::setLoopState(int state)
{
    if(state == 0){
        toggleLoopButton->setIcon(QIcon(":/res/img/no_loop"));
        playerState->loop = false;
    } else if(state == 1) {
        toggleLoopButton->setIcon(QIcon(":/res/img/loop"));
        playerState->loop = true;
    }
}

void CasterPlayerWidget::setAudioDuckState(int state)
{
    if(state == 0){
        isAudioDucked = false;
        player->setVolume(playerState->volume);
    } else if(state == 1) {
       isAudioDucked = true;
       player->setVolume(playerState->volume * 0.33);
    }
    this->update();
}

//==================================================

//Protected Methods
//=================DRAG_N_DROP=============================
void CasterPlayerWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void CasterPlayerWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void CasterPlayerWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void CasterPlayerWidget::dropEvent(QDropEvent *event)
{
        const QMimeData* mimeData = event->mimeData();

        if (mimeData->hasUrls())
        {
            QStringList pathList;
            QList<QUrl> urlList = mimeData->urls();

            for (int i = 0; i < urlList.size() && i < 32; ++i)
            {
                pathList.append(urlList.at(i).toLocalFile());
            }

            if(openFiles(pathList))
            {
                event->acceptProposedAction();
                //playSound();
            }
            else
            {
                 QMessageBox msgBox;
                 msgBox.setText("CasterSoundboard does not accept this file type.\nMake sure your system has the necessary codecs installed.\nCasterSoundboard can also play the audio from videos.");
                 msgBox.setInformativeText("Try: (*.mp3), (*.wav), (*.ogg), (*.flac), (*.m4a).\nAnd Try: (*.mp4), (*.mov), (*.ogv), (*.avi), (*.mpg), (*.wmv).");
                 msgBox.setStandardButtons(QMessageBox::Close);
                 msgBox.setDefaultButton(QMessageBox::Close);
                 msgBox.setModal(true);
                 int ret = msgBox.exec();
                 Q_UNUSED(ret);
            }
        }
}

bool CasterPlayerWidget::openFiles(const QStringList& pathList)
{
    //Determine File Type
    QFileInfo fi(pathList[0]);

    if(fi.suffix() == "mp3" ||
            fi.suffix() == "wav" ||
            fi.suffix() == "flac" ||
            fi.suffix() == "ogg" ||
            fi.suffix() == "m4a" ||
            fi.suffix() == "mp4" ||
            fi.suffix() == "mov" ||
            fi.suffix() == "ogv" ||
            fi.suffix() == "avi" ||
            fi.suffix() == "mpg" ||
            fi.suffix() == "wmv")
    {
        soundFilePath = new QString(pathList[0]);//Sets File Path
        volumeSlider->setValue(playerState->volume);
        player->setMedia(QUrl::fromLocalFile(soundFilePath->toUtf8().constData()));
        newMediaLoaded = true;

        //Update Player Save State
        playerState->filePath = soundFilePath; // Save file path to player state

        return true;
    }
    else
    {
        return false;
    }
}

//=========================================================

//===========Mouse Press/Touch Event===========
/* When user touches wiget it toggles the player state (Play/Pause/Stop) */
void CasterPlayerWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    // Toggle play state
    // Play/Pause
    //CURRENT PLAY STATE TOGGLE LOGIC
    volumeSlider->setValue(playerState->volume);
    if(player->state() == QMediaPlayer::PlayingState)
    {
        player->pause();
    }
    else if (player->state() == QMediaPlayer::StoppedState)
    {
        player->setPosition(playerState->startTime);
        player->play();
    }
    else if(player->state() == QMediaPlayer::PausedState)
    {
        player->play();
    }
}

//=========================================================

//===========Focus Event===========
void CasterPlayerWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    this->setFocus();
}

//===========Paint Event===========
void CasterPlayerWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    //Make widget render it's own style sheet.
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);


    // Set Progressbar Dimensions
    QLinearGradient lgBrush(0,0,0,this->height());
    QRect rec(0,0,this->getProgressWidth(),this->height());

    // Draw Progressbar
    if(player->state() == QMediaPlayer::PlayingState)
    {
        lgBrush.setColorAt(0.0,QColor(184,225,252,255));
        lgBrush.setColorAt(0.1,QColor(169,210,243,255));
        lgBrush.setColorAt(0.25,QColor(144,186,228,255));
        lgBrush.setColorAt(0.37,QColor(144,188,234,255));
        lgBrush.setColorAt(0.5,QColor(144,191,240,255));
        lgBrush.setColorAt(0.51,QColor(107,168,229,255));
        lgBrush.setColorAt(0.83,QColor(162,218,245,255));
        lgBrush.setColorAt(1.0,QColor(189,243,253,255));

        playStateImage->load(":/res/img/playState_playing.png");
    }
    else if(player->state() == QMediaPlayer::PausedState)
    {
        lgBrush.setColorAt(0.0,QColor(243,252,70,255));
        lgBrush.setColorAt(0.1,QColor(226,244,60,255));
        lgBrush.setColorAt(0.25,QColor(206,235,41,255));
        lgBrush.setColorAt(0.37,QColor(206,239,41,255));
        lgBrush.setColorAt(0.5,QColor(210,243,41,255));
        lgBrush.setColorAt(0.51,QColor(200,232,16,255));
        lgBrush.setColorAt(0.83,QColor(246,243,56,255));
        lgBrush.setColorAt(1.0,QColor(253,217,75,255));

        playStateImage->load(":/res/img/playState_paused.png");
    }
    else if (player->state() == QMediaPlayer::StoppedState)
    {
        lgBrush.setColorAt(0.0,QColor(252,70,79,255));
        lgBrush.setColorAt(0.1,QColor(244,60,60,255));
        lgBrush.setColorAt(0.25,QColor(235,50,41,255));
        lgBrush.setColorAt(0.37,QColor(239,54,41,255));
        lgBrush.setColorAt(0.5,QColor(243,54,41,255));
        lgBrush.setColorAt(0.51,QColor(232,27,16,255));
        lgBrush.setColorAt(0.83,QColor(246,56,78,255));
        lgBrush.setColorAt(1.0,QColor(253,75,128,255));

        playStateImage->load(":/res/img/playState_stopped.png");
    }
    p.fillRect(rec,lgBrush);

    // Draw Play State
    p.drawImage(this->width()/2 - playStateImage->width()/2, this->height()/2 - playStateImage->height()/2, *playStateImage);

    // Draw Audio Ducking Indicator
    if(isAudioDucked){
        playStateImage->load(":/res/img/33_percent.png");
        p.drawImage(this->width()/2 - playStateImage->scaled(64,64).width()/2, this->height()/2 - playStateImage->scaled(64,64).height()/2, playStateImage->scaled(64,64));
    }

}
//====================================

//==========Reload From Player State==========
void CasterPlayerWidget::reloadFromPlayerState()
{
    /* Reload Colors */
    colorPicker->set_slider_H_Value(playerState->slider_H_Value);
    colorPicker->set_slider_S_Value(playerState->slider_S_Value);
    colorPicker->set_slider_L_Value(playerState->slider_L_Value);
    if(playerState->PlayerBackgroundCSS->toUtf8() != "")
    {
        this->setStyleSheet(playerState->PlayerBackgroundCSS->toUtf8());
    }
    /* Reload Loop UI */
    if(playerState->loop){
        toggleLoopButton->setIcon(QIcon(":/res/img/loop"));
    }
    /* Reload Player Settings */
    QStringList pathList;
    pathList.append(playerState->filePath->toUtf8());
    newMediaLoadedFromProfile = true;
    openFiles(pathList);
    volumeSlider->setValue(playerState->volume);
    player->setPosition(playerState->startTime);

}


//========================================================
//==========OSC Composer Methods=====
OscMessageComposer* CasterPlayerWidget::writeOSCMessage(QString address, int value){
    // Compose OSC Message
    OscMessageComposer* msg = new OscMessageComposer(address);
    msg->pushInt32((qint32)value);
    return msg;
}

OscMessageComposer* CasterPlayerWidget::writeOSCMessage(QString address, float value){
    // Compose OSC Message
    OscMessageComposer* msg = new OscMessageComposer(address);
    msg->pushFloat(value);
    return msg;
}

OscMessageComposer* CasterPlayerWidget::writeOSCMessage(QString address, QString value){
    // Compose OSC Message
    OscMessageComposer* msg = new OscMessageComposer(address);
    msg->pushString(value.toUtf8());
    return msg;
}
