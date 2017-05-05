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
#include "CSS.h"
#include <QSizePolicy>
#include <QString>
#include <QFile>
#include <QFileDialog>
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
#include <QPainterPath>
#include <QBrush>
#include <QLinearGradient>
#include <QGraphicsDropShadowEffect>

//Constructor
CasterPlayerWidget::CasterPlayerWidget(QWidget* parent) : QWidget(parent)
{
    //Set Widget Defaults
    this->setAcceptDrops(true);

    //Init Player
    state = NoFile;
    player = new QMediaPlayer();
    //Init Properties
    soundFilePath = new QString("");
    hotKeyLetter = new QString("1");
    progress = 0.0;
    volume = 100;

    //Set-Up Widget Layout
    soundNameLabel = new QLabel("<Drop File or click to choose>");
    soundNameLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    QFont sNLF("Georgia",7,-1,false); sNLF.setBold(true);
    soundNameLabel->setFont(sNLF);
    soundNameLabel->setWordWrap(true);
    //soundNameLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    hotKeyLabel = new QLabel(*hotKeyLetter);
    hotKeyLabel->setAlignment(Qt::AlignCenter);
    hotKeyLabel->setFont(QFont("Georgia",13,-1,false));
    hotKeyLabel->setStyleSheet("background:url(:/res/img/Key.png) no-repeat;background-attachment:fixed;background-position:center;color:white;");
    //hotKeyLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    timeLabel = new QLabel("-00:00");
    timeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignRight);
    QFont tLF("Georgia",8,-1,false); tLF.setBold(true);
    timeLabel->setFont(tLF);
    playStateButton = new QPushButton("");
    playStateButton->setIcon(QIcon(":/res/img/stop.png"));
    playStateButton->setIconSize(QSize(16,16));
    playStateButton->setFlat(true);
    //playStateButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    subMenuButton = new QPushButton("");
    subMenuButton->setIcon(QIcon(":/res/img/down.png"));
    subMenuButton->setIconSize(QSize(16,16));
    subMenuButton->setFlat(true);
    //subMenuButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    volumeSlider = new QSlider(Qt::Vertical);
    volumeSlider->setValue(100);
    volumeSlider->setStyleSheet("QSlider:vertical {"
                                    "min-width: 30px;"
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

    bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(playStateButton);
    bottomLayout->addWidget(subMenuButton);


    subMainLayoutV->addLayout(topLayout);
    subMainLayoutV->addLayout(centerLayout);
    subMainLayoutV->addLayout(bottomLayout);

    subMainLayoutH->addLayout(subMainLayoutV);
    subMainLayoutH->addWidget(volumeSlider);

    mainLayout->addLayout(subMainLayoutH);

    //Widget Styling
    this->setMouseTracking(true);
    this->setStyleSheet(CSS_GRAIENT_GREY);
    QGraphicsDropShadowEffect * dse = new QGraphicsDropShadowEffect();
    dse->setBlurRadius(10);
    this->setGraphicsEffect(dse);

    //Connect Sub-Widget Events
    connect(playStateButton,SIGNAL(clicked()),this,SLOT(playerToggle()));
    connect(volumeSlider,SIGNAL(valueChanged(int)),this,SLOT(volumeChanged(int)));
    connect(subMenuButton,SIGNAL(clicked()),this,SLOT(openSubMenu()));
    connect(player,SIGNAL(positionChanged(qint64)),this,SLOT(playerPositionChanged(qint64)));
    connect(player,SIGNAL(stateChanged(QMediaPlayer::State)),this,SLOT(playerStateChanged(QMediaPlayer::State)));
    connect(player,SIGNAL(metaDataChanged()),this,SLOT(playerMetaDataChanged()));
    connect(player,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),this,SLOT(playerNewMediaStatus(QMediaPlayer::MediaStatus)));
    connect(player,SIGNAL(error(QMediaPlayer::Error)),this,SLOT(playerError(QMediaPlayer::Error)));
}

//Set Properties
void CasterPlayerWidget::setHotKeyLetter(QString hotKey)
{
    hotKeyLabel->setText(hotKey);
}

//----SIGNALS----

//-----SLOTS----
void CasterPlayerWidget::playerToggle()
{
    //CURRENT PLAY STATE TOGGLE LOGIC
    player->setVolume(volume);
    if(player->state() == QMediaPlayer::PlayingState)
    {
        player->pause();
    }
    else if (player->state() == QMediaPlayer::StoppedState)
    {
        player->play();
    }
    else if(player->state() == QMediaPlayer::PausedState)
    {
        player->play();
    }
}

void CasterPlayerWidget::volumeChanged(int value)
{
    //Update volume change
    volume = value;
    player->setVolume(volume);
}

void CasterPlayerWidget::openSubMenu()
{
    //OPEN SUB MENU
    CasterLabelColorPicker colorPicker(this);
    colorPicker.exec();
    if(colorPicker.ok)
    {
        this->setStyleSheet(colorPicker.currentCSS);
    }
}

void CasterPlayerWidget::playerPositionChanged(qint64 position)
{
    if(player->duration() > 0)
    {
        progress = (float)(position) / (float)(player->duration());
        int timeLeft = player->duration() - position;
        int seconds = (int) (timeLeft / 1000) % 60 ;
        int minutes = (int) ((timeLeft / (1000*60)) % 60);
        //int hours   = (int) ((timeLeft / (1000*60*60)) % 24);
        QString timeRemaining = "-" + QString("%1").arg(minutes,2,'i',0,'0') + ":" + QString("%1").arg(seconds,2,'i',0,'0');
        timeLabel->setText(timeRemaining);
        this->update();
    }
}

void CasterPlayerWidget::playerStateChanged(QMediaPlayer::State state)
{
    //CURRENT PLAY STATE
    if(state == QMediaPlayer::PlayingState)
    {
        playStateButton->setIcon(QIcon(":/res/img/play.png"));
    }
    else if(state == QMediaPlayer::PausedState)
    {
        playStateButton->setIcon(QIcon(":/res/img/pause.png"));
    }
    else if (state == QMediaPlayer::StoppedState)
    {
        playStateButton->setIcon(QIcon(":/res/img/stop.png"));

        progress = 0;
        int timeLeft = player->duration();
        int seconds = (int) (timeLeft / 1000) % 60 ;
        int minutes = (int) ((timeLeft / (1000*60)) % 60);
        QString timeRemaining = "-" + QString("%1").arg(minutes,2,'i',0,'0') + ":" + QString("%1").arg(seconds,2,'i',0,'0');
        timeLabel->setText(timeRemaining);
        this->update();
    }

}

void CasterPlayerWidget::playerMetaDataChanged()
{
    //Update Meta Data
    int timeLeft = player->duration();
    int seconds = (int) (timeLeft / 1000) % 60 ;
    int minutes = (int) ((timeLeft / (1000*60)) % 60);
    QString timeRemaining = "-" + QString("%1").arg(minutes,2,'i',0,'0') + ":" + QString("%1").arg(seconds,2,'i',0,'0');
    timeLabel->setText(timeRemaining);
}

void CasterPlayerWidget::playerNewMediaStatus(QMediaPlayer::MediaStatus status)
{
    // If we are already in Error state, don't try to update the label at all.
    if (state == Error)
        return;

    if (status == QMediaPlayer::LoadingMedia) {
        soundNameLabel->setText("Loading...");
        soundNameLabel->setStyleSheet("color:darkblue;");
        player->stop();
        state = Loading;
    } else if (status != QMediaPlayer::InvalidMedia) {
        // We want to change the label on all occasions except if we have
        // InvalidMedia, where we also get an Error signal which is handled in
        // playerError().
        QFileInfo fi(*soundFilePath);
        QString text;
        if (player->metaData(QMediaMetaData::Title).toString() != "")
            //Use metadata title
            text = player->metaData(QMediaMetaData::Title).toString();
        else //Use filename as title
            text = QFileInfo(*soundFilePath).baseName();
        soundNameLabel->setText(text);
        soundNameLabel->setStyleSheet("");
        state = Active;
    }
}

void CasterPlayerWidget::playerError(QMediaPlayer::Error error)
{
    if (error == QMediaPlayer::NoError)
        return;

    soundNameLabel->setText("Error: " + player->errorString());
    soundNameLabel->setStyleSheet("color:red;");
    player->stop();
    state = Error;
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
    player->setVolume(volume);
    player->play();
}

void CasterPlayerWidget::stopSound()
{
    player->setVolume(volume);
    player->stop();
}

bool CasterPlayerWidget::assignFile(const QString &path)
{
    if(openFiles(QStringList(path)))
    {
        return true;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("CasterSoundboard does not accept this file type.\nMake sure your system has the necessary codecs installed.\nCasterSoundboard can also play the audio from videos.");
        msgBox.setInformativeText("Try: (*.mp3), (*.wav), (*.ogg), (*.flac), (*.m4a).\nAnd Try: (*.mp4), (*.mov), (*.ogv), (*.avi), (*.mpg), (*.wmv).");
        msgBox.setStandardButtons(QMessageBox::Close);
        msgBox.setDefaultButton(QMessageBox::Close);
        msgBox.setModal(true);
        msgBox.exec();
        return false;
    }

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
        QString path = mimeData->urls().at(0).toLocalFile();
        if (assignFile(path)) {
            playSound();
            event->acceptProposedAction();
        }
    }
}

void CasterPlayerWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    event->accept();
    QString path = QFileDialog::getOpenFileName(
        this, "Open audio file", "",
        "Audio files (*.mp3 *.wav *.ogg *.flac *.m4a);;"
        "Video files (*.mp4 *.mov *.ogv *.avi *.mpg *.wmv)"
    );
    if (!path.isNull() && assignFile(path))
        playSound();
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
        // Put the state into NoFile here, so that we're not going to be stuck
        // in the Error state forever.
        state = NoFile;
        player->setVolume(volume);
        player->setMedia(QUrl::fromLocalFile(soundFilePath->toUtf8().constData()));
        return true;
    }
    else
    {
        return false;
    }
}

//=========================================================

//===========Focus Event===========
void CasterPlayerWidget::mouseMoveEvent(QMouseEvent *event)
{
    this->setFocus();
}

//===========Paint Event===========
void CasterPlayerWidget::paintEvent(QPaintEvent *event)
{
    //Make widget render it's own style sheet.
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);


    //Draw Progressbar
    QLinearGradient lgBrush(0,0,0,this->height());
    QRect rec(0,0,this->getProgressWidth(),this->height());
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
    }
    p.fillRect(rec,lgBrush);

}
//====================================
