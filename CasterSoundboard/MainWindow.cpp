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
#include "MainWindow.h"
#include "CasterBoard.h"
#include "CasterPlayer.h"
#include "CasterPlayerState.h"
#include <QObject>
#include <QMessageBox>
#include <QInputDialog>
#include <QToolBar>
#include <QPushButton>
#include <QTableWidget>
#include <QGridLayout>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QDataStream>
#include <QFileDialog>
#include <QStatusBar>
#include <QUdpSocket>
#include <QByteArray>
#include "libs/osc/reader/OscReader.h"
#include "libs/osc/reader/OscMessage.h"
#include "libs/osc/reader/OscBundle.h"
#include "libs/osc/composer/OscMessageComposer.h"
#include "libs/osc/OscPatternMatching.h"
#include "libs/osc/reader/types/OscAddress.h"
#include "libs/osc/reader/types/OscValue.h"



//CONSTRUCTOR
MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    // UDP Socket
    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::Any,5051);
    connect(socket, SIGNAL(readyRead()), this, SLOT(executeOSCCommand()));

    //SET WINDOW PROPETIES
    this->setWindowTitle("CasterSoundboard");
    this->setWindowIcon(QIcon(":/res/img/app.png"));

    //DENFINE LAYOUT
    QGridLayout *layout = new QGridLayout;

    //INIT NEW TAB PUSH BUTTONS
    //~~New Tab~~
    addNewTabButton = new QPushButton;
    addNewTabButton->setIcon(QIcon(":/res/img/newTab.png"));
    addNewTabButton->setIconSize(QSize(40,40));
    //addNewTabButton->setFixedSize(30,30);
    //layout->addWidget(addNewTabButton,0,2, Qt::AlignRight);

    //~~Open Tab~~
    openTabButton = new QPushButton;
    openTabButton->setIcon(QIcon(":/res/img/open.png"));
    openTabButton->setIconSize(QSize(40,40));
    //openTabButton->setFixedSize(30,30);

    //~~Save Tab~~
    saveTabButton = new QPushButton;
    saveTabButton->setIcon(QIcon(":/res/img/save.png"));
    saveTabButton->setIconSize(QSize(40,40));
    //saveTabButton->setFixedSize(30,30);

    //~~Save As Tab~~
    saveAsTabButton = new QPushButton;
    saveAsTabButton->setIcon(QIcon(":/res/img/save_as.png"));
    saveAsTabButton->setIconSize(QSize(40,40));
    //saveAsTabButton->setFixedSize(40,40);

    //~~Stop ALL Sounds~~
    stopAllSoundsButton = new QPushButton;
    stopAllSoundsButton->setIcon(QIcon(":/res/img/stopAll.png"));
    stopAllSoundsButton->setIconSize(QSize(40,40));

    //~~Toogle Ducking~~
    toggleAudioDuckingButton = new QPushButton;
    toggleAudioDuckingButton->setIcon(QIcon(":/res/img/duck.png"));
    toggleAudioDuckingButton->setIconSize(QSize(40,40));

    //~~Rename Tab~~
    renameCurrentTabButton = new QPushButton;
    renameCurrentTabButton->setIcon(QIcon(":/res/img/rename.png"));
    renameCurrentTabButton->setIconSize(QSize(40,40));

    //~~OSC Settings~~
    openSoundControlButton = new QPushButton;
    openSoundControlButton->setIcon(QIcon(":/res/img/open_sound_control.png"));
    openSoundControlButton->setIconSize(QSize(40,40));

    //~~About Button~~
    aboutButton = new QPushButton;
    aboutButton->setIcon(QIcon(":/res/img/about.png"));
    aboutButton->setIconSize(QSize(40,40));
    //aboutButton->setFixedSize(30,30);
    //layout->addWidget(aboutButton,0,0, Qt::AlignLeft);

    //======Main Toolbar=========
    mainToolbar = new QToolBar;
    // Add buttons
    mainToolbar->addWidget(addNewTabButton);
    mainToolbar->addWidget(openTabButton);
    mainToolbar->addWidget(saveTabButton);
    mainToolbar->addWidget(saveAsTabButton);
    mainToolbar->addWidget(renameCurrentTabButton);
    mainToolbar->addWidget(stopAllSoundsButton);
    mainToolbar->addWidget(toggleAudioDuckingButton);
    mainToolbar->addWidget(openSoundControlButton);
    mainToolbar->addWidget(aboutButton);
    // Add toolbar to layout
    layout->addWidget(mainToolbar, 0, 0, Qt::AlignLeft);

    //INIT MAIN TAB CONTAINER
    mainTabContainer = new QTabWidget;
    mainTabContainer->setTabsClosable(true);
    layout->addWidget(mainTabContainer,1,0, 2, 0);

    // Status Bar
    main_statusbar = new QStatusBar;
    main_statusbar->setMaximumHeight(15);
    layout->addWidget(main_statusbar);

    //SET LAYOUT
    this->setLayout(layout);

    //MAKE CONNECTIONS
    connect(aboutButton,SIGNAL(clicked()),this,SLOT(aboutBox()));
    connect(addNewTabButton,SIGNAL(clicked()),this,SLOT(addNewTab()));
    connect(mainTabContainer,SIGNAL(tabCloseRequested(int)),this,SLOT(mainTabContainerTabClosedRequested(int)));
    connect(saveTabButton,SIGNAL(clicked()),this,SLOT(saveTab()));
    connect(saveAsTabButton,SIGNAL(clicked()),this,SLOT(saveAsTab()));
    connect(openTabButton,SIGNAL(clicked()),this,SLOT(openProfile()));
    connect(renameCurrentTabButton,SIGNAL(clicked()),this,SLOT(renameCurrentTab()));
    connect(stopAllSoundsButton,SIGNAL(clicked()),this,SLOT(stopAllSounds()));
    connect(toggleAudioDuckingButton,SIGNAL(clicked()),this,SLOT(toggleAudioDucking()));
    connect(openSoundControlButton, SIGNAL(clicked()), this, SLOT(openOSCSettings()) );

    // Properties

}

//SLOTS
void MainWindow::aboutBox()
{
    QMessageBox msgBox;
    msgBox.setText("CasterSoundboard (v1.0) BETA\nAuthor: Oscar Cerna\ne-mail: covarianttensor@gmail.com\nLicense: LGPL v3\n© 2013-2017 Oscar Cerna");
    msgBox.setInformativeText("Special Note:\nDeveloped for Chris Fisher & Jupiter Broadcasting, because he's awesome!\nVisit www.jupiterbroadcasting.com.\nNOT AFFILIATED with Chris Fisher or Jupiter Broadcasting.");
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.setDefaultButton(QMessageBox::Close);
    msgBox.setModal(true);
    msgBox.setWindowTitle("About");
    msgBox.setWindowIcon(QIcon(":/res/img/about.png"));
    msgBox.exec();
}

void MainWindow::addNewTab()
{
    //ASK FOR SOUNDBOARD TAB
    bool ok;
    QString text = QInputDialog::getText(
                          this,
                          tr("String"),
                          tr("Enter a tab name:"),
                          QLineEdit::Normal,
                          tr(""),
                          &ok );
    if(ok)
    {
        //CREATE REQUESTED TABS
        CasterBoard *cb = new CasterBoard;
        if(text != "")
        {
            cb->soundBoardName = new QString(text);
            mainTabContainer->addTab(cb, text);
        }
        else
        {
            mainTabContainer->addTab(cb, "No Name");
        }

    }

}

void MainWindow::mainTabContainerTabClosedRequested(int tabIndex)
{
    //CONFIRM TAB CLOSE
    QMessageBox msgBox;
    msgBox.setText("Are you sure you want to close this tab");
    msgBox.setWindowIcon(QIcon(":/res/img/app.png"));
    msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setModal(true);
    if(msgBox.exec() == QMessageBox::Yes)
    {
        //CLOSE REQUESTED TAB
        mainTabContainer->removeTab(tabIndex);
    }
}

void MainWindow::saveTab()
{
    if(mainTabContainer->count() > 0)
    {
        if(dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->profileFilePath->toUtf8() != ""){
            /* Save */
            QFile file;
            QDataStream out;

            file.setFileName(dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->profileFilePath->toUtf8());
            file.open(QIODevice::WriteOnly);
            out.setDevice(&file);
            out.setVersion(QDataStream::Qt_DefaultCompiledVersion);
            out << *dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget());
            file.flush();
            file.close();
        } else {
            /* Save As.. */
            QString _filePath = QFileDialog::getSaveFileName(this, "Save Sound Board Profile As...", "",
                                                             "Sound Board Profile (*.caster)");

            if (!_filePath.isNull())
            {
                /* Save Profile As... */
                QFile file;
                QDataStream out;

                file.setFileName(_filePath.toUtf8() + ".caster");
                file.open(QIODevice::WriteOnly);
                out.setDevice(&file);
                out.setVersion(QDataStream::Qt_DefaultCompiledVersion);
                out << *dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget());
                file.flush();
                file.close();

                //Set Save Path
                dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->profileFilePath = new QString(_filePath.toUtf8() + ".caster");
            }
        }
    }

}

void MainWindow::saveAsTab()
{
    //Debug
    /*
    QString *temp = dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->player1->playerState->filePath;
    QMessageBox msgBox;
    msgBox.setText("File Path: " + temp->toUtf8() );
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.setDefaultButton(QMessageBox::Close);
    msgBox.setModal(true);
    msgBox.setWindowTitle("About");
    msgBox.setWindowIcon(QIcon(":/res/img/about.png"));
    msgBox.exec();
    */

    if(mainTabContainer->count() > 0)
    {
        QString _filePath = QFileDialog::getSaveFileName(this, "Save Sound Board Profile As...", "",
                                                         "Sound Board Profile (*.caster)");

        if (!_filePath.isNull())
        {
            /* Save Profile As... */
            QFile file;
            QDataStream out;

            file.setFileName(_filePath.toUtf8() + ".caster");
            file.open(QIODevice::WriteOnly);
            out.setDevice(&file);
            out.setVersion(QDataStream::Qt_DefaultCompiledVersion);
            out << *dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget());
            file.flush();
            file.close();

            //Set Save Path
            dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->profileFilePath = new QString(_filePath.toUtf8() + ".caster");
        }
    }



}

void MainWindow::openProfile()
{
    //File Diag
    QString _filePath = QFileDialog::getOpenFileName(
            this, "Open Sound Board Profile", "",
            "Sound Board Profile (*.caster)");

    if (!_filePath.isNull())
    {
        //Load Profile Data
        CasterBoard *cb = new CasterBoard(this);
        QFile file;
        QDataStream in;
        file.setFileName(_filePath);
        file.open(QIODevice::ReadOnly);
        in.setDevice(&file);
        in.setVersion(QDataStream::Qt_DefaultCompiledVersion);
        in >> *cb;
        file.close();

        //Set Save Path
        *cb->profileFilePath = _filePath;

        // Load Profile Into New Board
        cb->reloadBoardFromPlayerStates();
        mainTabContainer->addTab(cb, cb->soundBoardName->toUtf8());
    }

}

void MainWindow::stopAllSounds()
{
    // Iterate through boards
    for(int i = 0; i < mainTabContainer->count(); i++)
    {
        dynamic_cast<CasterBoard*>(mainTabContainer->widget(i))->stopAllSounds();
    }
}

void MainWindow::toggleAudioDucking()
{
    if(audio_duck_state == 0){
        audio_duck_state = 1;
        toggleAudioDuckingButton->setIcon(QIcon(":/res/img/unduck.png"));
    } else {
        audio_duck_state = 0;
        toggleAudioDuckingButton->setIcon(QIcon(":/res/img/duck.png"));
    }
    // Iterate through boards
    for(int i = 0; i < mainTabContainer->count(); i++)
    {
        dynamic_cast<CasterBoard*>(mainTabContainer->widget(i))->setAllAudioDuckingStates(audio_duck_state);
    }

    // Give current board keyboard focus again
    if(mainTabContainer->count() > 0 )
        dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->setFocus();
}

void MainWindow::renameCurrentTab()
{
    if(mainTabContainer->count() > 0){
        //ASK FOR SOUNDBOARD TAB
        bool ok;
        QString text = QInputDialog::getText(
                              this,
                              tr("String"),
                              tr("Enter a new tab name:"),
                              QLineEdit::Normal,
                              tr(""),
                              &ok );
        if(ok)
        {
            //CREATE REQUESTED TABS
            if(text != "")
            {
                mainTabContainer->setTabText(mainTabContainer->currentIndex(), text);
                dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->soundBoardName = new QString(text);
            }

        }
    }

}

void MainWindow::openOSCSettings()
{
    // Send OSC Message
    /*
    OscMessageComposer msg( "/1/boardName");
    msg.pushString("Hi Dude!");
    QByteArray* formattedMsg = msg.getBytes();
    QHostAddress oscClientAddressVal("192.168.1.96");
    socket->writeDatagram(*formattedMsg, oscClientAddressVal, 9000);
    */
    //this->setFocus();
}

// OSC Server

void MainWindow::executeOSCCommand()
{
    // Get UDP Datagram //
    QByteArray datagram;
    datagram.resize(socket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;
    socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Read OSC Packet //
    OscReader reader(&datagram);
    // Check if direct message (we are ignoring bundles)
    if(reader.getMessage() != 0){
        // Get Message //
        OscMessage* msg = reader.getMessage();
        QString address = msg->getAddress();
        QStringList address_params = address.split("/");
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        // Print OSC Command //
        QString values("");
        for(int i = 0; i < msg->getNumValues(); i++){
            if(i == 0){ values = msg->getValue(i)->toString(); } else { values += " | " + msg->getValue(i)->toString(); }
        }
        main_statusbar->showMessage("[OSC Message] Address: " + address.toUtf8() + " Value(s): " + values.toUtf8() , 500);
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        // Resolve OSC Address //
        // Is One-Way Address
        if(address_params.value(1) == "castersoundboard")
        {
            // Is Board Address
            if(address_params.value(2) == "board")
            {
                // Is A Board Name
                if(address_params.value(3) != 0 && address_params.value(3) != "")
                {
                    //==================================
                    // Check If Board Exists
                    bool _boardWasFound = false;
                    int _board_index = 0;
                    for(int i=0; i < mainTabContainer->count(); i++){
                        // Is Board Name Found?
                        if(dynamic_cast<CasterBoard*>(mainTabContainer->widget(i))->soundBoardName == address_params.value(3))
                        {
                            _boardWasFound = true;
                            _board_index = i;
                            break;
                        }
                    }
                    // Finish if board was not found
                    if(!_boardWasFound)
                        return;//Board Not Found
                    //==================================

                    // Is Player Address
                    if(address_params.value(4) == "player")
                    {
                        // Is Player Name
                        if(address_params.value(5) != 0 && address_params.value(5) != "")
                        {
                            //==================================
                            // Check If Player Does NOT Exist
                            if(!dynamic_cast<CasterBoard*>(mainTabContainer->widget(_board_index))->players->contains(address_params.value(5))){
                                delete msg; return;//Player does not exist on board.
                            }

                            QString _player_key(address_params.value(5));
                            //==================================

                            // Is Action Address
                            if(address_params.value(6) == "modify")
                            {
                                // Is Interface Address
                                //~~~~Execute Decoded Command~~~~
                                if(address_params.value(7) == "volume"
                                        && msg->getNumValues() == 1
                                        && address_params.value(8) == 0){
                                    try{// Perform Action
                                        int _volume = 100 * msg->getValue(0)->toFloat();
                                        dynamic_cast<CasterBoard*>(mainTabContainer->widget(_board_index))->players->value(_player_key)->volumeSlider->setValue(_volume);
                                    } catch(...) {}// Type Casting Exception Most Likely
                                } else if (address_params.value(7) == "track_position"
                                           && msg->getNumValues() == 1
                                           && address_params.value(8) == 0){
                                    try{// Perform Action
                                        int _position = 100 * msg->getValue(0)->toFloat();
                                        dynamic_cast<CasterBoard*>(mainTabContainer->widget(_board_index))->players->value(_player_key)->trackBar->setValue(_position);
                                    } catch(...) {}// Type Casting Exception Most Likely
                                } else if (address_params.value(7) == "play_state"
                                           && msg->getNumValues() == 1
                                           && address_params.value(8) != 0){// Is Player State Address
                                    if(address_params.value(8) == "play" && address_params.value(9) == 0){
                                        try{// Perform Action
                                            if(msg->getValue(0)->toInteger() == 1)
                                                dynamic_cast<CasterBoard*>(mainTabContainer->widget(_board_index))->players->value(_player_key)->playSound();
                                        } catch(...){}// Type Casting Exception Most Likely
                                    } else if(address_params.value(8) == "pause" && address_params.value(9) == 0){
                                        try{// Perform Action
                                            if(msg->getValue(0)->toInteger() == 1)
                                                dynamic_cast<CasterBoard*>(mainTabContainer->widget(_board_index))->players->value(_player_key)->pauseSound();
                                        } catch(...){}// Type Casting Exception Most Likely
                                    } else if(address_params.value(8) == "stop" && address_params.value(9) == 0){
                                        try{// Perform Action
                                            if(msg->getValue(0)->toInteger() == 1)
                                                dynamic_cast<CasterBoard*>(mainTabContainer->widget(_board_index))->players->value(_player_key)->stopSound();
                                        } catch(...){}// Type Casting Exception Most Likely
                                    }
                                } else if (address_params.value(7) == "loop_state"
                                           && msg->getNumValues() == 1
                                           && address_params.value(8) == 0){
                                    try{// Perform Action (0 = no loop, 1 = loop)
                                        dynamic_cast<CasterBoard*>(mainTabContainer->widget(_board_index))->players->value(_player_key)->setLoopState(msg->getValue(0)->toInteger());
                                    } catch(...){}// Type Casting Exception Most Likely
                                }
                                //~~~~Execute Decoded Command End~~~~
                                delete msg; return;//EXIT
                            }//END_IF ACTION

                        }//END_IF PLAYER NAME

                    }//END_IF PLAYER

                }//END_IF BOARD NAME

            }//END_IF BOARD ADDRESS

        }//END_IF CASTERSOUNDBOARD
        delete msg; return;//EXIT
    }//END_IF DIRECT MESSAGE
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

}



// PROTECTED
void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Shift){
        // Toogle Audio Ducking //
        this->toggleAudioDucking();
    }
}
