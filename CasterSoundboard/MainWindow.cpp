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
#include <QNetworkInterface>
#include "CasterOSCServerConfigPicker.h"
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

    // OSC Config Picker
    outbound_ipv4 = new QString("0.0.0.0");
    oscConfigPicker = new CasterOSCServerConfigPicker("0.0.0.0", inbound_port, *outbound_ipv4, outbound_port);

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
    addNewTabButton->setToolTip("New Tab");
    //addNewTabButton->setFixedSize(30,30);
    //layout->addWidget(addNewTabButton,0,2, Qt::AlignRight);

    //~~Open Tab~~
    openTabButton = new QPushButton;
    openTabButton->setIcon(QIcon(":/res/img/open.png"));
    openTabButton->setIconSize(QSize(40,40));
    openTabButton->setToolTip("Open Tab");
    //openTabButton->setFixedSize(30,30);

    //~~Save Tab~~
    saveTabButton = new QPushButton;
    saveTabButton->setIcon(QIcon(":/res/img/save.png"));
    saveTabButton->setIconSize(QSize(40,40));
    saveTabButton->setToolTip("Save Tab");
    //saveTabButton->setFixedSize(30,30);

    //~~Save As Tab~~
    saveAsTabButton = new QPushButton;
    saveAsTabButton->setIcon(QIcon(":/res/img/save_as.png"));
    saveAsTabButton->setIconSize(QSize(40,40));
    saveAsTabButton->setToolTip("Save Tab As");
    //saveAsTabButton->setFixedSize(40,40);

    //~~Stop ALL Sounds~~
    stopAllSoundsButton = new QPushButton;
    stopAllSoundsButton->setIcon(QIcon(":/res/img/stopAll.png"));
    stopAllSoundsButton->setIconSize(QSize(40,40));
    stopAllSoundsButton->setToolTip("Stop All Sounds");

    //~~Toogle Ducking~~
    toggleAudioDuckingButton = new QPushButton;
    toggleAudioDuckingButton->setIcon(QIcon(":/res/img/duck.png"));
    toggleAudioDuckingButton->setIconSize(QSize(40,40));
    toggleAudioDuckingButton->setToolTip("Toggle Ducking");

    //~~Rename Tab~~
    renameCurrentTabButton = new QPushButton;
    renameCurrentTabButton->setIcon(QIcon(":/res/img/rename.png"));
    renameCurrentTabButton->setIconSize(QSize(40,40));
    renameCurrentTabButton->setToolTip("Rename Tab");

    //~~OSC Settings~~
    openSoundControlButton = new QPushButton;
    openSoundControlButton->setIcon(QIcon(":/res/img/open_sound_control.png"));
    openSoundControlButton->setIconSize(QSize(40,40));
    openSoundControlButton->setToolTip("OSC Settings");

    //~~About Button~~
    aboutButton = new QPushButton;
    aboutButton->setIcon(QIcon(":/res/img/about.png"));
    aboutButton->setIconSize(QSize(40,40));
    aboutButton->setToolTip("About");
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
    connect(mainTabContainer,SIGNAL(currentChanged(int)),this,SLOT(currentTabWasChanged(int)));
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
        disconnect(dynamic_cast<CasterBoard*>(mainTabContainer->widget(tabIndex)), SIGNAL(globalHotKeyReleasedEvent(QKeyEvent*)),this,SLOT(handleGlobalHotKeyEventFromCurrentWidget(QKeyEvent*)));
        disconnect(dynamic_cast<CasterBoard*>(mainTabContainer->widget(tabIndex)), SIGNAL(_updateOSCClient(OscMessageComposer)),this,SLOT(sendOSCMessageToClient(OscMessageComposer)));
        mainTabContainer->removeTab(tabIndex);
    }
}

void MainWindow::currentTabWasChanged(int tabIndex)
{
    this->updateCurrentOSCTab(tabIndex);
}

void MainWindow::updateCurrentOSCTab(int tabIndex)
{
    // Update Current Board & Previous Board Status
    if(0 <= previous_tab_index
            && previous_tab_index < mainTabContainer->count()
            && previous_tab_index != tabIndex){
        //Keep track in tab indicies
        dynamic_cast<CasterBoard*>(mainTabContainer->widget(previous_tab_index))->isCurrentBoard = false;
        dynamic_cast<CasterBoard*>(mainTabContainer->widget(tabIndex))->isCurrentBoard = true;
        previous_tab_index = tabIndex;

        // Update OSC Client
        dynamic_cast<CasterBoard*>(mainTabContainer->widget(tabIndex))->syncWithOSCClient();
    } else if(0 <= previous_tab_index
              && previous_tab_index < mainTabContainer->count()
              && previous_tab_index == tabIndex){
        //Update Current tab (even if redundant)
        dynamic_cast<CasterBoard*>(mainTabContainer->widget(tabIndex))->isCurrentBoard = true;

        // Update OSC Client
        dynamic_cast<CasterBoard*>(mainTabContainer->widget(tabIndex))->syncWithOSCClient();
    }
}

void MainWindow::aboutBox()
{
    QMessageBox msgBox;
    msgBox.setText("CasterSoundboard (v1.0) BETA<br>Author: <a href=""https://github.com/covarianttensor"">Oscar Cerna</a><br>e-mail: <a href=mailto:covarianttensor@gmail.com>covarianttensor@gmail.com</a><br>License: LGPL v3<br>© 2013-2017 Oscar Cerna");
    msgBox.setInformativeText("Special Note:<br>Developed for Chris Fisher & Jupiter Broadcasting, because he's awesome!<br>Visit <a href=""http://www.jupiterbroadcasting.com"">jupiterbroadcasting.com</a>.<br>NOT AFFILIATED with Chris Fisher or Jupiter Broadcasting.");
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
        cb->setAllAudioDuckingStates(audio_duck_state);
        connect(cb, SIGNAL(globalHotKeyReleasedEvent(QKeyEvent*)),this,SLOT(handleGlobalHotKeyEventFromCurrentWidget(QKeyEvent*)));
        connect(cb, SIGNAL(_updateOSCClient(OscMessageComposer*)),this,SLOT(sendOSCMessageToClient(OscMessageComposer*)));

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
        cb->setAllAudioDuckingStates(audio_duck_state);
        connect(cb, SIGNAL(globalHotKeyReleasedEvent(QKeyEvent*)),this,SLOT(handleGlobalHotKeyEventFromCurrentWidget(QKeyEvent*)));
        connect(cb, SIGNAL(_updateOSCClient(OscMessageComposer*)),this,SLOT(sendOSCMessageToClient(OscMessageComposer*)));
        mainTabContainer->addTab(cb, cb->soundBoardName->toUtf8());
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

void MainWindow::stopAllSounds()
{
    // Iterate through boards
    for(int i = 0; i < mainTabContainer->count(); i++)
    {
        dynamic_cast<CasterBoard*>(mainTabContainer->widget(i))->stopAllSounds();
    }

    // Give current board keyboard focus again
    if(mainTabContainer->count() > 0 )
        dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->setFocus();
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

    //Update OSC Client
    OscMessageComposer* msg = writeOSCMessage("/glo/m/audio_d_s", audio_duck_state);
    this->sendOSCMessageToClient(msg);
}

// Tab Switching
void MainWindow::switchToNextTab()
{
    if(mainTabContainer->count() > 0){
        int next_tab_index = mainTabContainer->currentIndex() + 1;
        if(next_tab_index < mainTabContainer->count()){
            // Change Tabs
            mainTabContainer->setCurrentIndex(next_tab_index);
            mainTabContainer->currentWidget()->setFocus();//Give Keyboard Focus (Hot Keying)

            //Update OSC Client
            dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->syncWithOSCClient();
        }
    }
}

void MainWindow::switchToPrevTab()
{
    if(mainTabContainer->count() > 0){
        int prev_tab_index = mainTabContainer->currentIndex() - 1;
        if(prev_tab_index >= 0){
            // Change Tabs
            mainTabContainer->setCurrentIndex(prev_tab_index);
            mainTabContainer->currentWidget()->setFocus();//Give Keyboard Focus (Hot Keying)

            //Update OSC Client
            dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->syncWithOSCClient();
        }
    }
}


// Board Signa Emissions Handlers
void MainWindow::hotKeyExecution(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Shift){
        // Toogle Audio Ducking //
        this->toggleAudioDucking();
    } else if(event->key() == Qt::Key_Space){
        // Stop All Sounds //
        this->stopAllSounds();
    }
}

void MainWindow::handleGlobalHotKeyEventFromCurrentWidget(QKeyEvent *event)
{
    this->hotKeyExecution(event);
}

// PROTECTED
void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    this->hotKeyExecution(event);
}

// OSC Server

void MainWindow::openOSCSettings()
{
    // Set Local IP
    if(get_local_ip() != 0){
        QString* ip = new QString(get_local_ip().toUtf8());
        oscConfigPicker->setInBoundIPv4(ip);
    }

    // Run Seeting Diag
    oscConfigPicker->exec();
    if(oscConfigPicker->ok == true)
    {
        if(oscConfigPicker->Result == CasterOSCServerConfigPicker::Action_Start_Server){
            // Attempt To Start Server
            inbound_port = oscConfigPicker->getInboundPort();
            outbound_ipv4 = new QString(oscConfigPicker->getOutboundIPv4().toUtf8());
            outbound_port = oscConfigPicker->getOutboundPort();
            bool bind_success = socket->bind(QHostAddress::Any, inbound_port);
            if(bind_success){
                // Notify User
                QString *temp = new QString("Success!! OSC server running on port " + QString::number(inbound_port).toUtf8());
                QMessageBox msgBox;
                msgBox.setText(temp->toUtf8() + "\n");
                msgBox.setStandardButtons(QMessageBox::Close);
                msgBox.setDefaultButton(QMessageBox::Close);
                msgBox.setModal(true);
                msgBox.setWindowTitle("Connection Successful");
                msgBox.setWindowIcon(QIcon(":/res/img/about.png"));
                msgBox.exec();

                // Connect Listener Server
                connect(socket, SIGNAL(readyRead()), this, SLOT(executeOSCCommand()));

                //Update Diag
                oscConfigPicker->setIsServerRunning(true);

                // Reset Result
                oscConfigPicker->ok = false;
                oscConfigPicker->Result = CasterOSCServerConfigPicker::Action_Neutral;

                // Synchronize UI Info
                if(mainTabContainer->count() > 0 ){
                    dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->syncWithOSCClient();
                }
                this->syncWithOSCClient();
            } else {
                QString *temp = new QString("Sorry, inbound port " + QString::number(inbound_port).toUtf8() + " appears to be in use.\nPlease try another port." );
                QMessageBox msgBox;
                msgBox.setText(temp->toUtf8() + "\n");
                msgBox.setStandardButtons(QMessageBox::Close);
                msgBox.setDefaultButton(QMessageBox::Close);
                msgBox.setModal(true);
                msgBox.setWindowTitle("Port Connection Error");
                msgBox.setWindowIcon(QIcon(":/res/img/about.png"));
                msgBox.exec();

                //Update Diag
                oscConfigPicker->setIsServerRunning(false);

                // Reset Result
                oscConfigPicker->ok = false;
                oscConfigPicker->Result = CasterOSCServerConfigPicker::Action_Neutral;
            }

        } else if (oscConfigPicker->Result == CasterOSCServerConfigPicker::Action_Kill_Server ){
            // Disconnect Listener Server
            disconnect(socket, SIGNAL(readyRead()), this, SLOT(executeOSCCommand()));
            // Unbind socket from port
            socket->close();

            //Update Diag
            oscConfigPicker->setIsServerRunning(false);

            // Reset Result
            oscConfigPicker->ok = false;
            oscConfigPicker->Result = CasterOSCServerConfigPicker::Action_Neutral;
        }

    }
}

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
        for(int i = 0; i < (int)msg->getNumValues(); i++){
            if(i == 0){ values = msg->getValue(i)->toString(); } else { values += " | " + msg->getValue(i)->toString(); }
        }
        main_statusbar->showMessage("[OSC Message] Address: " + address.toUtf8() + " Value(s): " + values.toUtf8() , 500);
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        // Resolve OSC Address //
        if(address_params.value(1) == "castersoundboard")
        {
            // Is One-Way Address
            this->executeOneWayOSCCommand(msg, address_params);
        }//END_IF CASTERSOUNDBOARD
        else {//TWO WAY ADDRESS
            this->executeTwoWayOSCCommand(msg, address_params);
        }
        delete msg; return;//EXIT
    }//END_IF DIRECT MESSAGE
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

}

void MainWindow::executeOneWayOSCCommand(OscMessage *msg, QStringList address_params)
{
    // Route OSC Message //
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
                        return;//Player does not exist on board.
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
                        } else if (address_params.value(7) == "track_position_percent"
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
                            } else if(address_params.value(8) == "resume" && address_params.value(9) == 0){
                                try{// Perform Action
                                    if(msg->getValue(0)->toInteger() == 1)
                                        dynamic_cast<CasterBoard*>(mainTabContainer->widget(_board_index))->players->value(_player_key)->resumeSound();
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
                        return;//EXIT
                    }//END_IF ACTION

                }//END_IF PLAYER NAME

            }//END_IF PLAYER

        }//END_IF BOARD NAME

        //END_IF BOARD ADDRESS
    } else if(address_params.value(2) == "global"){
        if(address_params.value(3) == "modify"){
            // Is Interface Address
            //~~~~Execute Decoded Command~~~~
            if (address_params.value(4) == "all_play_states"
                       && msg->getNumValues() == 1
                       && address_params.value(5) != 0){// Is Global Player States Address
                if(address_params.value(5) == "stop" && address_params.value(6) == 0){
                    try{// Perform Action
                        if(msg->getValue(0)->toInteger() == 1)
                            this->stopAllSounds();
                    } catch(...){}// Type Casting Exception Most Likely
                }
            } else if (address_params.value(4) == "audio_ducking_state"
                       && msg->getNumValues() == 1
                       && address_params.value(5) == 0){
                try{// Perform Action (0 = no duck, 1 = duck)
                    audio_duck_state = msg->getValue(0)->toInteger();
                    if(audio_duck_state == 0){
                        toggleAudioDuckingButton->setIcon(QIcon(":/res/img/duck.png"));
                    } else {
                        audio_duck_state = 1;
                        toggleAudioDuckingButton->setIcon(QIcon(":/res/img/unduck.png"));
                    }

                    // Iterate through boards
                    for(int i = 0; i < mainTabContainer->count(); i++)
                    {
                        dynamic_cast<CasterBoard*>(mainTabContainer->widget(i))->setAllAudioDuckingStates(audio_duck_state);
                    }

                    // Give current board keyboard focus again
                    if(mainTabContainer->count() > 0 )
                        dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->setFocus();
                } catch(...){}// Type Casting Exception Most Likely
            }
            //~~~~Execute Decoded Command End~~~~
            return;//EXIT
        }
    }//END_ELSE_IF GLOBAL ADDRESS
}

void MainWindow::executeTwoWayOSCCommand(OscMessage *msg, QStringList address_params)
{
    // Route OSC Message //
    // Is Current Board Player Address
    if(address_params.value(1) == "cbp")// cbp = Current Board Player
    {
        //==================================
        // Check If Current Board Does NOT Exist
        if(mainTabContainer->count() <= 0)
            return;//No current board exists.

        // Is A Player Name
        if(address_params.value(2) != 0 && address_params.value(2) != "")
        {
            //==================================
            // Check If Player Does NOT Exist
            if(!dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->players->contains(address_params.value(2)))
                return;//Player does not exist on current board.
            QString _player_key(address_params.value(2));
            //==================================

            // Is Action Address
            if(address_params.value(3) == "m")// m = modify
            {
                // Is Interface Address
                //~~~~Execute Decoded Command~~~~
                if(address_params.value(4) == "vol"
                        && msg->getNumValues() == 1
                        && address_params.value(5) == 0){//vol = volume
                    try{// Perform Action
                        int _volume = 100 * msg->getValue(0)->toFloat();
                        dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->players->value(_player_key)->volumeSlider->setValue(_volume);
                    } catch(...) {}// Type Casting Exception Most Likely
                } else if (address_params.value(4) == "t_p_p"
                           && msg->getNumValues() == 1
                           && address_params.value(5) == 0){//t_p_p = track_position_percent
                    try{// Perform Action
                        int _position = 100 * msg->getValue(0)->toFloat();
                        dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->players->value(_player_key)->trackBar->setValue(_position);
                    } catch(...) {}// Type Casting Exception Most Likely
                } else if (address_params.value(4) == "p_s"
                           && msg->getNumValues() == 1
                           && address_params.value(5) != 0){//p_s = play_state // Is Player State Address
                    if(address_params.value(5) == "play_stop" && address_params.value(6) == 0){//
                        try{// Perform Action
                            if(msg->getValue(0)->toInteger() == 1)
                                dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->players->value(_player_key)->play_stop_toggle();
                        } catch(...){}// Type Casting Exception Most Likely
                    } else if(address_params.value(5) == "resume_pause" && address_params.value(6) == 0){
                        try{// Perform Action
                            if(msg->getValue(0)->toInteger() == 1)
                                dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->players->value(_player_key)->resume_pause_toggle();
                        } catch(...){}// Type Casting Exception Most Likely
                    }
                } else if (address_params.value(4) == "l_s"
                           && msg->getNumValues() == 1
                           && address_params.value(5) == 0){//l_s = loop_state
                    try{// Perform Action (0 = no loop, 1 = loop)
                        dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->players->value(_player_key)->setLoopState(msg->getValue(0)->toInteger());
                    } catch(...){}// Type Casting Exception Most Likely
                }
                //~~~~Execute Decoded Command End~~~~
                return;//EXIT
            }//END_IF ACTION ADDRESS

        }//END_IF PLAYER NAME

        //END_IF CURRENT BOARD PLAYER ADDRESS
    } else if(address_params.value(1) == "glo"){//glo = global
        if(address_params.value(2) == "m"){//m = modify
            // Is Interface Address
            //~~~~Execute Decoded Command~~~~
            if (address_params.value(3) == "all_p_s"
                       && msg->getNumValues() == 1
                       && address_params.value(4) != 0){//all_p_s = all_play_states // Is Global Player States Address
                if(address_params.value(4) == "stop" && address_params.value(5) == 0){
                    try{// Perform Action
                        if(msg->getValue(0)->toInteger() == 1)
                            this->stopAllSounds();
                    } catch(...){}// Type Casting Exception Most Likely
                }
            } else if (address_params.value(3) == "audio_d_s"
                       && msg->getNumValues() == 1
                       && address_params.value(4) == 0){//audio_d_s = audio_ducking_state
                try{// Perform Action (0 = no duck, 1 = duck)
                    audio_duck_state = msg->getValue(0)->toInteger();
                    if(audio_duck_state == 0){
                        toggleAudioDuckingButton->setIcon(QIcon(":/res/img/duck.png"));
                    } else {
                        audio_duck_state = 1;
                        toggleAudioDuckingButton->setIcon(QIcon(":/res/img/unduck.png"));
                    }

                    // Iterate through boards
                    for(int i = 0; i < mainTabContainer->count(); i++)
                    {
                        dynamic_cast<CasterBoard*>(mainTabContainer->widget(i))->setAllAudioDuckingStates(audio_duck_state);
                    }

                    // Give current board keyboard focus again
                    if(mainTabContainer->count() > 0 )
                        dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->setFocus();
                } catch(...){}// Type Casting Exception Most Likely
            } else if (address_params.value(3) == "current_tab"
                       && msg->getNumValues() == 1
                       && address_params.value(4) != 0){
                if(address_params.value(4) == "next" && address_params.value(5) == 0){
                    try{// Perform Action
                        if(msg->getValue(0)->toInteger() == 1)
                            this->switchToNextTab();
                    } catch(...){}// Type Casting Exception Most Likely
                } else if(address_params.value(4) == "prev" && address_params.value(5) == 0){
                    try{// Perform Action
                        if(msg->getValue(0)->toInteger() == 1)
                            this->switchToPrevTab();
                    } catch(...){}// Type Casting Exception Most Likely
                }
            }
            //~~~~Execute Decoded Command End~~~~
            return;//EXIT
        } else if(address_params.value(2) == "sync"){// Triggers Client UI Syncing
            try{// Perform Action
                if(msg->getValue(0)->toInteger() == 1)
                    // Synchronize UI Info
                    if(mainTabContainer->count() > 0 ){
                        this->syncWithOSCClient();
                        dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->syncWithOSCClient();
                    }
            } catch(...){}// Type Casting Exception Most Likely
        }
    }//END_ELSE_IF GLOBAL ADDRESS
}

void MainWindow::sendOSCMessageToClient(OscMessageComposer* message)
{
    // Send OSC Message
    try{
        if(oscConfigPicker->getIsServerRunning()){
            QByteArray* oscDatagram = message->getBytes();
            QHostAddress oscClientAddress(*outbound_ipv4);
            socket->writeDatagram(*oscDatagram, oscClientAddress, outbound_port);
        }
    } catch(...) { }

    delete message;
}

//========================================================
//==========OSC Composer Methods=====
OscMessageComposer* MainWindow::writeOSCMessage(QString address, int value){
    // Compose OSC Message
    OscMessageComposer* msg = new OscMessageComposer(address);
    msg->pushInt32((qint32)value);
    return msg;
}

OscMessageComposer* MainWindow::writeOSCMessage(QString address, float value){
    // Compose OSC Message
    OscMessageComposer* msg = new OscMessageComposer(address);
    msg->pushFloat(value);
    return msg;
}

OscMessageComposer* MainWindow::writeOSCMessage(QString address, QString value){
    // Compose OSC Message
    OscMessageComposer* msg = new OscMessageComposer(address);
    msg->pushString(value.toUtf8());
    return msg;
}

//=========================================================
//===Utility Methods====
QString MainWindow::get_local_ip(){
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
             return address.toString();
    }
    return 0;
}

void MainWindow::syncWithOSCClient(){
    //Update Audio Ducking
    OscMessageComposer* msg = writeOSCMessage("/glo/m/audio_d_s", audio_duck_state);
    this->sendOSCMessageToClient(msg);
}
