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
    player = new CasterMediaPlayer(this);
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
    cuePicker = new CasterCuePicker(0, 0, this);
    colorPicker = new CasterLabelColorPicker(this);

    //Internal Properties
    newMediaLoaded = false;
    newMediaLoadedFromProfile = false;

    //Set-Up Widget Layout
    soundNameLabel = new QLabel("<Drop File>");
    soundNameLabel->setStyleSheet("background-color:transparent;color:black;");
    soundNameLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    QFont sNLF = soundNameLabel->font();
    sNLF.setPointSize(10);
    sNLF.setBold(true);
    soundNameLabel->setFont(sNLF);
    soundNameLabel->setWordWrap(true);
    soundNameLabel->setMaximumWidth(150);
    //soundNameLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    hotKeyLabel = new QLabel(*hotKeyLetter);
    hotKeyLabel->setAlignment(Qt::AlignCenter);
    QFont hKLF = hotKeyLabel->font();
    hKLF.setPointSize(13);
    hotKeyLabel->setFont(hKLF);
    hotKeyLabel->setFixedSize(36, 36);
    hotKeyLabel->setStyleSheet("background-image:url(:/res/img/Key.png);background-repeat:no-repeat;background-position:center;color:white;");
    //hotKeyLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    timeLabel = new QLabel("+00:00\n-00:00");
    timeLabel->setStyleSheet("background-color:transparent;color:black;");
    timeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    QFont tLF = timeLabel->font();
    tLF.setPointSize(9);
    tLF.setBold(true);
    timeLabel->setFont(tLF);
    /* Play Button */
    playStateButton = new QPushButton("", this);
    playStateButton->setIcon(QIcon(":/res/img/stop.png"));
    playStateButton->setIconSize(QSize(18,18));
    playStateButton->setFlat(true);
    playStateButton->hide();
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
    toggleLoopButton->setIcon(QIcon(":/res/img/no_loop.png"));
    toggleLoopButton->setIconSize(QSize(28,28));
    toggleLoopButton->setFlat(true);
    /* Sub Menu Button */
    colorPickerButton = new QPushButton("");
    colorPickerButton->setIcon(QIcon(":/res/img/colorPicker.png"));
    colorPickerButton->setIconSize(QSize(16,16));
    colorPickerButton->setFlat(true);
    //colorPickerButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    /* Edit Notes Button */
    editNotesButton = new QPushButton("", this);
    editNotesButton->setIcon(QIcon(":/res/img/notes.png"));
    editNotesButton->setIconSize(QSize(28,28));
    editNotesButton->setFlat(true);
    editNotesButton->hide();
    /* Volume Slider */
    volumeSlider = new QSlider(Qt::Vertical);
    volumeSlider->setRange(0, 100);
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
    trackBar->setRange(0, 100);
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
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(4);

    subMainLayoutH = new QHBoxLayout;
    subMainLayoutH->setContentsMargins(0, 0, 0, 0);
    subMainLayoutH->setSpacing(4);
    subMainLayoutV = new QVBoxLayout;
    subMainLayoutV->setContentsMargins(0, 0, 0, 0);
    subMainLayoutV->setSpacing(2);

    topLayout = new QHBoxLayout;
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(hotKeyLabel);
    topLayout->addWidget(timeLabel);

    centerLayout = new QHBoxLayout;
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->addWidget(soundNameLabel);

    /* Layout: Bottom Player Buttons */
    bottomLayout_TopButtons = new QHBoxLayout;
    bottomLayout_TopButtons->setContentsMargins(0, 0, 0, 0);
    //bottomLayout_TopButtons->addWidget(playStateButton);
    bottomLayout_TopButtons->addWidget(openFileButton);
    bottomLayout_TopButtons->addWidget(setCueButton);
    bottomLayout_TopButtons->addWidget(toggleLoopButton);

    bottomLayout_BottomButtons = new QHBoxLayout;
    bottomLayout_BottomButtons->setContentsMargins(0, 0, 0, 0);
    bottomLayout_BottomButtons->addWidget(colorPickerButton);
    //bottomLayout_BottomButtons->addWidget(editNotesButton);

    bottomLayout = new QVBoxLayout;
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(2);
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

    //Connect Sub-Widget Events
    connect(playStateButton,SIGNAL(clicked()),this,SLOT(playerToggle()));
    connect(volumeSlider,SIGNAL(valueChanged(int)),this,SLOT(volumeChanged(int)));
    connect(trackBar,SIGNAL(valueChanged(int)),this,SLOT(trackBarChanged(int)));
    connect(openFileButton,SIGNAL(clicked()),this,SLOT(openFileDiag()));
    connect(colorPickerButton,SIGNAL(clicked()),this,SLOT(openColorPicker()));
    connect(setCueButton,SIGNAL(clicked()),this,SLOT(openCuePicker()));
    connect(toggleLoopButton,SIGNAL(clicked()),this,SLOT(toggleLooping()));
    connect(player, &QMediaPlayer::positionChanged, this, &CasterPlayerWidget::playerPositionChanged, Qt::QueuedConnection);
    connect(player, &QMediaPlayer::playbackStateChanged, this, &CasterPlayerWidget::playerStateChanged);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &CasterPlayerWidget::playerMediaStatusChanged, Qt::QueuedConnection);
    connect(player, &QMediaPlayer::metaDataChanged, this, &CasterPlayerWidget::playerMetaDataChanged);
    connect(player, &QMediaPlayer::durationChanged, this, &CasterPlayerWidget::playerDurationChanged);
}

CasterPlayerWidget::~CasterPlayerWidget()
{
    player->stop();
    player->setMedia(QUrl());
    delete playStateImage;
    delete playerState;
    delete soundFilePath;
    delete hotKeyLetter;
    delete id;
}

//Set Properties
void CasterPlayerWidget::setHotKeyLetter(QString hotKey)
{
    hotKeyLabel->setText(hotKey);
    *this->hotKeyLetter = hotKey;
    *this->id = hotKey.toUtf8();
}

QString CasterPlayerWidget::displayColor()
{
    // Let Qt interpret the applied stylesheet, including fractional RGB values
    // in existing profiles. Sliders are adjustments, not absolute HSL colours.
    ensurePolished();
    // Hidden/reloaded widgets can still have the previous stylesheet palette.
    style()->polish(this);
    const QBrush background = palette().brush(QPalette::Window);
    const QGradient *gradient = background.gradient();
    if (gradient && !gradient->stops().isEmpty())
        return gradient->stops().first().second.name(QColor::HexRgb).toUpper();
    // Defensive fallback for missing/unsupported profile styling.
    return QStringLiteral("#DADADA");
}

void CasterPlayerWidget::syncWithOSCClient()
{
    const QString baseAddress = "/cbp/" + *this->id + "/m";

    //Update Volume
    emit updateOSCClient(writeOSCMessage(baseAddress + "/vol", static_cast<float>(volumeSlider->value() / 100.0f)));

    //Update Track Position
    emit updateOSCClient(writeOSCMessage(baseAddress + "/t_p_p", static_cast<float>(trackBar->value() / 100.0f)));

    //Update Title
    emit updateOSCClient(writeOSCMessage(baseAddress + "/label/tr_name", this->soundNameLabel->text()));

    //Update Loop State
    emit updateOSCClient(writeOSCMessage(baseAddress + "/l_s", static_cast<int>(playerState->loop)));

    //Update Time
    const QStringList times = this->timeLabel->text().split("\n");
    emit updateOSCClient(writeOSCMessage(baseAddress + "/label/time", times.value(0) + "/" + times.value(1)));

    //Update Play State
    if(player->state() == QMediaPlayer::PlayingState){
        emit updateOSCClient(writeOSCMessage(baseAddress + "/label/p_s", "Playing"));
    }
    else if(player->state() == QMediaPlayer::PausedState){
        emit updateOSCClient(writeOSCMessage(baseAddress + "/label/p_s", "Paused"));
    }
    else if (player->state() == QMediaPlayer::StoppedState){
        emit updateOSCClient(writeOSCMessage(baseAddress + "/label/p_s", "Stopped"));
    }

    emit updateOSCClient(writeOSCMessage(baseAddress + "/color", displayColor()));
}

//----SIGNALS----

//-----SLOTS----
void CasterPlayerWidget::playerToggle()
{
    play_stop_toggle();
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
        qint64 position = (value * player->duration()) / 100;
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
    const QString previousColor = displayColor();
    colorPicker->set_slider_H_Value(playerState->slider_H_Value);
    colorPicker->set_slider_S_Value(playerState->slider_S_Value);
    colorPicker->set_slider_L_Value(playerState->slider_L_Value);
    // Cancel, Escape and the window close button must not reuse a prior OK.
    colorPicker->ok = false;

    colorPicker->exec();
    if(colorPicker->ok)
    {
        this->setStyleSheet(colorPicker->currentCSS);
        // Update Player State
        *playerState->PlayerBackgroundCSS = colorPicker->currentCSS;
        playerState->backgroundCSSChanged = true;
        playerState->slider_H_Value = colorPicker->slider_H_Value;
        playerState->slider_S_Value = colorPicker->slider_S_Value;
        playerState->slider_L_Value = colorPicker->slider_L_Value;
        const QString currentColor = displayColor();
        if (currentColor != previousColor)
            emit updateOSCClient(writeOSCMessage("/cbp/" + *id + "/m/color", currentColor));

    }
}

void CasterPlayerWidget::openCuePicker()
{
    if (playerState->filePath->isEmpty() || player->duration() <= 0)
        return;

    cuePicker->setFilePath(playerState->filePath, (int)player->duration(), playerState->loop);
    cuePicker->setVolume(playerState->volume);
    if(playerState->timeSet){
        cuePicker->setStopTime(playerState->stopTime);
        cuePicker->setStartTime(playerState->startTime);
    } else {
        cuePicker->setStopTime((int)player->duration());
        cuePicker->setStartTime(playerState->startTime);
    }
    cuePicker->ok = false;

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
    setLoopState(playerState->loop ? 0 : 1);

    //Update OSC Client
    OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/l_s", playerState->loop ? 1 : 0);
    emit updateOSCClient(msg);
}

void CasterPlayerWidget::validateCueRange(qint64 duration)
{
    if (duration <= 0)
        return; // Loading can temporarily report an unknown duration.
    if (!playerState->timeSet || playerState->startTime < 0
            || playerState->startTime >= duration
            || playerState->stopTime <= playerState->startTime) {
        playerState->startTime = 0;
        playerState->stopTime = duration;
        playerState->timeSet = false;
    } else if (playerState->stopTime > duration) {
        playerState->stopTime = duration;
    }
}

void CasterPlayerWidget::playerPositionChanged(qint64 position)
{
    // Queued notifications may describe a position from before the last seek.
    if (player->position() < position && player->mediaStatus() != QMediaPlayer::EndOfMedia)
        return;
    if(player->duration() > 0)
    {
        validateCueRange(player->duration());
        //Player State Update
        if(!playerState->timeSet && playerState->stopTime == 0){
            playerState->stopTime = player->duration();
        }

        const qint64 currentPosition = (player->mediaStatus() == QMediaPlayer::EndOfMedia)
                ? qMax(position, player->position())
                : player->position();
        const qint64 clampedPosition = qBound<qint64>(0, currentPosition, player->duration());

        //Current Time
        int currentTime_seconds = (int) (clampedPosition / 1000) % 60 ;
        int currentTime_minutes = (int) ((clampedPosition / (1000*60)) % 60);
        QString currentTime = "+" + QString("%1").arg(currentTime_minutes, 2, 10, QChar('0')) + ":" + QString("%1").arg(currentTime_seconds, 2, 10, QChar('0'));
        //Time Remaining
        progress = (float)(clampedPosition) / (float)(player->duration());
        int timeLeft = player->duration() - clampedPosition;
        int timeLeft_seconds = (int) (timeLeft / 1000) % 60 ;
        int timeLeft_minutes = (int) ((timeLeft / (1000*60)) % 60);
        //int hours   = (int) ((timeLeft / (1000*60*60)) % 24);
        QString timeRemaining = "-" + QString("%1").arg(timeLeft_minutes, 2, 10, QChar('0')) + ":" + QString("%1").arg(timeLeft_seconds, 2, 10, QChar('0'));
        timeLabel->setText(currentTime + "\n" + timeRemaining);// Set Time Remaining Label
        trackBarWasChangedByPlayer = true;
        trackBar->setValue((int)(progress * 100));
        trackBarWasChangedByPlayer = false;
        this->update();

        //Update OSC Client
        OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/time", currentTime + "/" + timeRemaining);
        emit updateOSCClient(msg);

        /* Enforce Start Time And Stop And Looping */
        // Loading/seeking a stopped player must never start a seek/stop cycle.
        // EndOfMedia is allowed so full-track loops can restart naturally.
        if (player->state() != QMediaPlayer::PlayingState
                && player->mediaStatus() != QMediaPlayer::EndOfMedia)
            return;
        if(currentPosition >= playerState->stopTime)
        {
            if(playerState->loop){
                player->setPosition(playerState->startTime);
                player->play();
            } else {
                player->stop();//make sure to use Qt::QueuedConnection
                player->setPosition(playerState->startTime);
            }
        }
        else if(currentPosition < playerState->startTime)
        {
            player->setPosition(playerState->startTime);
        }

    }

}

void CasterPlayerWidget::playerStateChanged(QMediaPlayer::PlaybackState state)
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
        if (playerState->loop && player->mediaStatus() == QMediaPlayer::EndOfMedia)
            return;
        playStateButton->setIcon(QIcon(":/res/img/stop.png"));

        //Update OSC Client
        OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/p_s", "Stopped");
        emit updateOSCClient(msg);

        progress = 0;
        int timeLeft = player->duration();
        int seconds = (int) (timeLeft / 1000) % 60 ;
        int minutes = (int) ((timeLeft / (1000*60)) % 60);
        QString timeRemaining = "+00:00\n-" + QString("%1").arg(minutes, 2, 10, QChar('0')) + ":" + QString("%1").arg(seconds, 2, 10, QChar('0'));
        timeLabel->setText(timeRemaining);
        this->update();

    }

}

void CasterPlayerWidget::playerMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status != QMediaPlayer::EndOfMedia
            || player->mediaStatus() != QMediaPlayer::EndOfMedia)
        return;
    validateCueRange(player->duration());
    if (playerState->loop) {
        player->setPosition(playerState->startTime);
        player->play();
    } else {
        player->stop();
        player->setPosition(playerState->startTime);
    }
}

void CasterPlayerWidget::playerMetaDataChanged()
{
    //Update Meta Data
    QFileInfo fi(*soundFilePath);
    const QString title = player->metaData().value(QMediaMetaData::Title).toString();
    if(title != "")
    {
        //Use metadata title
        soundNameLabel->setText(title);

        //Update OSC Client
        OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/tr_name", title);
        emit updateOSCClient(msg);
    }
    else if(!fi.baseName().isEmpty())
    {
        //Use filename as title
        soundNameLabel->setText(fi.baseName());

        //Update OSC Client
        OscMessageComposer* msg = writeOSCMessage("/cbp/" + this->id->toUtf8() + "/m/label/tr_name", fi.baseName());
        emit updateOSCClient(msg);
    }
}

void CasterPlayerWidget::playerDurationChanged(qint64 _duration)
{
    validateCueRange(_duration);
    if (_duration <= 0)
        return;
    if(newMediaLoaded && !newMediaLoadedFromProfile){
        //reset player state
        playerState->startTime = 0;
        playerState->stopTime = _duration;
        playerState->timeSet = false;
        newMediaLoaded = false;
        player->setPosition(playerState->startTime);
    } else if (newMediaLoadedFromProfile){
        newMediaLoaded = false;
        newMediaLoadedFromProfile = false;
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
    if (player->source().isEmpty())
        return;
    volumeSlider->setValue(playerState->volume);
    player->setPosition(playerState->startTime);
    player->play();
}

void CasterPlayerWidget::resumeSound()
{
    if (player->source().isEmpty())
        return;
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
    if(player->state() == QMediaPlayer::PlayingState)
    {
        stopSound();
    }
    else if (player->state() == QMediaPlayer::StoppedState
             || player->state() == QMediaPlayer::PausedState)
    {
        playSound();
    }
}

void CasterPlayerWidget::resume_pause_toggle()
{
    // Toggle play state
    // Play/Pause
    //CURRENT PLAY STATE TOGGLE LOGIC
    if(player->state() == QMediaPlayer::PlayingState)
    {
        pauseSound();
    }
    else if (player->state() == QMediaPlayer::StoppedState)
    {
        playSound();
    }
    else if(player->state() == QMediaPlayer::PausedState)
    {
        resumeSound();
    }
}

void CasterPlayerWidget::setLoopState(int state)
{
    if(state == 0){
        toggleLoopButton->setIcon(QIcon(":/res/img/no_loop.png"));
        playerState->loop = false;
    } else if(state == 1) {
        toggleLoopButton->setIcon(QIcon(":/res/img/loop.png"));
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
    if (pathList.isEmpty() || pathList[0].isEmpty())
        return false;

    //Determine File Type
    QFileInfo fi(pathList[0]);
    const QString suffix = fi.suffix().toLower();

    if(suffix == "mp3" ||
            suffix == "wav" ||
            suffix == "flac" ||
            suffix == "ogg" ||
            suffix == "m4a" ||
            suffix == "mp4" ||
            suffix == "mov" ||
            suffix == "ogv" ||
            suffix == "avi" ||
            suffix == "mpg" ||
            suffix == "wmv")
    {
        *soundFilePath = pathList[0];//Sets File Path
        //Update Player Save State
        *playerState->filePath = *soundFilePath; // Save file path to player state
        if (!newMediaLoadedFromProfile) {
            playerState->startTime = 0;
            playerState->stopTime = 0;
            playerState->timeSet = false;
        }
        soundNameLabel->setText(fi.baseName());
        newMediaLoaded = true;
        volumeSlider->setValue(playerState->volume);
        player->setMedia(QUrl::fromLocalFile(*soundFilePath));

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
    resume_pause_toggle();
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
    static const QImage playingImage(":/res/img/playState_playing.png");
    static const QImage pausedImage(":/res/img/playState_paused.png");
    static const QImage stoppedImage(":/res/img/playState_stopped.png");
    static const QImage duckedImage = QImage(":/res/img/33_percent.png").scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    //Make widget render it's own style sheet.
    QStyleOption opt;
    opt.initFrom(this);
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

        *playStateImage = playingImage;
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

        *playStateImage = pausedImage;
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

        *playStateImage = stoppedImage;
    }
    p.fillRect(rec,lgBrush);

    // Draw Play State
    p.drawImage(this->width()/2 - playStateImage->width()/2, this->height()/2 - playStateImage->height()/2, *playStateImage);

    // Draw Audio Ducking Indicator
    if(isAudioDucked){
        p.drawImage(this->width()/2 - duckedImage.width()/2, this->height()/2 - duckedImage.height()/2, duckedImage);
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
    this->setStyleSheet(playerState->PlayerBackgroundCSS->isEmpty()
                       ? QStringLiteral(CSS_GRAIENT_GREY) : *playerState->PlayerBackgroundCSS);
    /* Reload Loop UI */
    setLoopState(playerState->loop ? 1 : 0);
    /* Reload Player Settings */
    QStringList pathList;
    pathList.append(playerState->filePath->toUtf8());
    newMediaLoadedFromProfile = true;
    if (!openFiles(pathList)) {
        newMediaLoadedFromProfile = false;
    }
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
