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
#include <memory>
#include "CasterBoard.h"
#include "CasterPlayer.h"
#include "CasterPlayerState.h"
#include <QObject>
#include <QMessageBox>
#include <QInputDialog>
#include <QToolBar>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
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
#include <QGuiApplication>
#include <QStyleHints>
#include <QPalette>
#include <QEvent>
#include <QDebug>
#include <memory>
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
    oscConfigPicker = new CasterOSCServerConfigPicker("0.0.0.0", inbound_port, *outbound_ipv4, outbound_port, this);

    //SET WINDOW PROPETIES
    this->setWindowTitle("CasterSoundboard");
    this->setWindowIcon(QIcon(":/res/img/app.png"));
    //this->setStyleSheet("background-color:#4D4D4D;color:white");

    //DENFINE LAYOUT
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    //INIT NEW TAB PUSH BUTTONS
    //~~New Tab~~
    addNewTabButton = new QPushButton;
    addNewTabButton->setIconSize(QSize(40,40));
    addNewTabButton->setToolTip("New Tab");

    //~~Open Tab~~
    openTabButton = new QPushButton;
    openTabButton->setIconSize(QSize(40,40));
    openTabButton->setToolTip("Open Tab");

    //~~Save Tab~~
    saveTabButton = new QPushButton;
    saveTabButton->setIconSize(QSize(40,40));
    saveTabButton->setToolTip("Save Tab");

    //~~Save As Tab~~
    saveAsTabButton = new QPushButton;
    saveAsTabButton->setIconSize(QSize(40,40));
    saveAsTabButton->setToolTip("Save Tab As");

    //~~Stop ALL Sounds~~
    stopAllSoundsButton = new QPushButton;
    stopAllSoundsButton->setIconSize(QSize(40,40));
    stopAllSoundsButton->setToolTip("Stop All Sounds");

    //~~Toogle Ducking~~
    toggleAudioDuckingButton = new QPushButton;
    toggleAudioDuckingButton->setIconSize(QSize(40,40));
    toggleAudioDuckingButton->setToolTip("Toggle Ducking");

    //~~Rename Tab~~
    renameCurrentTabButton = new QPushButton;
    renameCurrentTabButton->setIconSize(QSize(40,40));
    renameCurrentTabButton->setToolTip("Rename Tab");

    //~~OSC Settings~~
    openSoundControlButton = new QPushButton;
    openSoundControlButton->setIconSize(QSize(40,40));
    openSoundControlButton->setToolTip("OSC Settings");

    //~~About Button~~
    aboutButton = new QPushButton;
    aboutButton->setIconSize(QSize(40,40));
    aboutButton->setToolTip("About");

    this->updateThemeIcons();

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
    layout->addWidget(mainToolbar, 0, Qt::AlignLeft);

    //INIT MAIN TAB CONTAINER
    mainTabContainer = new QTabWidget;
    mainTabContainer->setTabsClosable(true);
    layout->addWidget(mainTabContainer, 1);

    // Status Bar
    main_statusbar = new QStatusBar;
    main_statusbar->setSizeGripEnabled(false);
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (QGuiApplication::styleHints()) {
        connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
                this, [this](Qt::ColorScheme) { this->updateThemeIcons(); });
    }
#endif

    // Properties

}

MainWindow::~MainWindow()
{
    delete outbound_ipv4;
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
        CasterBoard *toBeDeleted = dynamic_cast<CasterBoard*>(mainTabContainer->widget(tabIndex));
        //CLOSE REQUESTED TAB
        if(toBeDeleted) {
            toBeDeleted->stopAllSounds();
            disconnect(toBeDeleted, SIGNAL(globalHotKeyReleasedEvent(QKeyEvent*)),this,SLOT(handleGlobalHotKeyEventFromCurrentWidget(QKeyEvent*)));
            disconnect(toBeDeleted, SIGNAL(_updateOSCClient(OscMessageComposer*)),this,SLOT(sendOSCMessageToClient(OscMessageComposer*)));
        }
        mainTabContainer->removeTab(tabIndex);
        delete toBeDeleted;
        if (mainTabContainer->count() > 0) {
            previous_tab_index = mainTabContainer->currentIndex();
            this->updateCurrentOSCTab(previous_tab_index);
        } else {
            previous_tab_index = 0;
        }
    }
}

void MainWindow::currentTabWasChanged(int tabIndex)
{
    this->updateCurrentOSCTab(tabIndex);
}

void MainWindow::updateCurrentOSCTab(int tabIndex)
{
    for (int i = 0; i < mainTabContainer->count(); ++i) {
        if (auto *board = dynamic_cast<CasterBoard*>(mainTabContainer->widget(i))) {
            board->isCurrentBoard = (i == tabIndex);
        }
    }
    previous_tab_index = (tabIndex >= 0 && tabIndex < mainTabContainer->count()) ? tabIndex : 0;

    if (auto *board = dynamic_cast<CasterBoard*>(mainTabContainer->widget(tabIndex))) {
        board->syncWithOSCClient();
    }
}

void MainWindow::aboutBox()
{
    QMessageBox msgBox;
    msgBox.setText("CasterSoundboard (v1.0) BETA<br>Author: <a href=""https://github.com/covarianttensor"">Oscar Cerna</a><br>e-mail: <a href=mailto:covarianttensor@gmail.com>covarianttensor@gmail.com</a><br>License: <a href=""https://github.com/JupiterBroadcasting/CasterSoundboard/blob/master/LICENSE"">LGPL v3</a><br>© 2013-2017 Oscar Cerna");
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
                          Q_NULLPTR,
                          tr("String"),
                          tr("Enter a tab name:"),
                          QLineEdit::Normal,
                          tr(""),
                          &ok );
    if(ok)
    {
        //CREATE REQUESTED TABS
        CasterBoard *cb = new CasterBoard(this);
        cb->setAllAudioDuckingStates(audio_duck_state);
        connect(cb, SIGNAL(globalHotKeyReleasedEvent(QKeyEvent*)),this,SLOT(handleGlobalHotKeyEventFromCurrentWidget(QKeyEvent*)));
        connect(cb, SIGNAL(_updateOSCClient(OscMessageComposer*)),this,SLOT(sendOSCMessageToClient(OscMessageComposer*)));

        if(text != "")
        {
            *cb->soundBoardName = text;
            mainTabContainer->addTab(cb, text);
        }
        else
        {
            mainTabContainer->addTab(cb, "No Name");
        }

    }

}

// Display paths and filesystem errors literally, without interpreting rich text.
static void showProfileError(QWidget *parent, const QString &message)
{
    QMessageBox box(QMessageBox::Warning, "CasterSoundboard", message,
                    QMessageBox::Ok, parent);
    box.setTextFormat(Qt::PlainText);
    box.exec();
}

void MainWindow::openProfile()
{
    const QString filePath = QFileDialog::getOpenFileName(
                this, "Open Sound Board Profile", "", "Sound Board Profile (*.caster)");
    if (filePath.isEmpty())
        return;

    QString error;
    if (!loadProfile(filePath, error))
        showProfileError(this, error);
}

bool MainWindow::loadProfile(const QString &filePath, QString &errorMessage)
{
    if (filePath.isEmpty()) {
        errorMessage = "No profile file specified.";
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Cannot open profile:\n" + filePath + "\n" + file.errorString();
        return false;
    }

    CasterBoard *cb = new CasterBoard(this);
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_DefaultCompiledVersion);
    in >> *cb;
    if (in.status() != QDataStream::Ok) {
        delete cb;
        errorMessage = "Cannot read profile:\n" + filePath;
        return false;
    }
    *cb->profileFilePath = filePath;
    cb->reloadBoardFromPlayerStates();
    cb->setAllAudioDuckingStates(audio_duck_state);
    connect(cb, SIGNAL(globalHotKeyReleasedEvent(QKeyEvent*)),this,SLOT(handleGlobalHotKeyEventFromCurrentWidget(QKeyEvent*)));
    connect(cb, SIGNAL(_updateOSCClient(OscMessageComposer*)),this,SLOT(sendOSCMessageToClient(OscMessageComposer*)));
    const int index = mainTabContainer->addTab(cb, *cb->soundBoardName);
    mainTabContainer->setCurrentIndex(index);
    return true;
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
            QString _filePath = QFileDialog::getSaveFileName(Q_NULLPTR, "Save Sound Board Profile As...", "",
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
                *dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->profileFilePath = _filePath.toUtf8() + ".caster";
            }
        }
    }

}

void MainWindow::saveAsTab()
{

    if(mainTabContainer->count() > 0)
    {
        QString _filePath = QFileDialog::getSaveFileName(Q_NULLPTR, "Save Sound Board Profile As...", "",
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
            *dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget())->profileFilePath = _filePath.toUtf8() + ".caster";
        }
    }



}

void MainWindow::renameCurrentTab()
{
    if(CasterBoard *board = currentBoard()){
        //ASK FOR SOUNDBOARD TAB
        bool ok;
        QString text = QInputDialog::getText(
                              Q_NULLPTR,
                              tr("String"),
                              tr("Enter a new tab name:"),
                              QLineEdit::Normal,
                              tr(""),
                              &ok );
        if(ok && !text.isEmpty())
        {
            mainTabContainer->setTabText(mainTabContainer->currentIndex(), text);
            *board->soundBoardName = text;
            sendOSCMessageToClient(writeOSCMessage("/glo/m/label/tab_name", text));
        }
    }

}

void MainWindow::stopAllSounds()
{
    // Iterate through boards
    for(int i = 0; i < mainTabContainer->count(); i++)
    {
        if (auto *board = dynamic_cast<CasterBoard*>(mainTabContainer->widget(i)))
            board->stopAllSounds();
    }

    // Give current board keyboard focus again
    if (CasterBoard *board = currentBoard())
        board->setFocus();
}

void MainWindow::setAudioDuckingState(int state, bool notifyOSC)
{
    audio_duck_state = (state == 0) ? 0 : 1;
    toggleAudioDuckingButton->setIcon(themedIcon(audio_duck_state == 0 ? "duck.png" : "unduck.png"));

    // Iterate through boards
    for (int i = 0; i < mainTabContainer->count(); i++) {
        if (auto *board = dynamic_cast<CasterBoard*>(mainTabContainer->widget(i)))
            board->setAllAudioDuckingStates(audio_duck_state);
    }

    // Give current board keyboard focus again
    if (CasterBoard *board = currentBoard())
        board->setFocus();

    if (notifyOSC) {
        OscMessageComposer* msg = writeOSCMessage("/glo/m/audio_d_s", audio_duck_state);
        this->sendOSCMessageToClient(msg);
    }
}

void MainWindow::toggleAudioDucking()
{
    setAudioDuckingState(audio_duck_state == 0 ? 1 : 0, true);
}

// Tab Switching
void MainWindow::switchToNextTab()
{
    if(mainTabContainer->count() > 0){
        int next_tab_index = mainTabContainer->currentIndex() + 1;
        if(next_tab_index < mainTabContainer->count()){
            // Change Tabs (currentChanged signal automatically triggers OSC sync)
            mainTabContainer->setCurrentIndex(next_tab_index);
            mainTabContainer->currentWidget()->setFocus();//Give Keyboard Focus (Hot Keying)
        }
    }
}

void MainWindow::switchToPrevTab()
{
    if(mainTabContainer->count() > 0){
        int prev_tab_index = mainTabContainer->currentIndex() - 1;
        if(prev_tab_index >= 0){
            // Change Tabs (currentChanged signal automatically triggers OSC sync)
            mainTabContainer->setCurrentIndex(prev_tab_index);
            mainTabContainer->currentWidget()->setFocus();//Give Keyboard Focus (Hot Keying)
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

void MainWindow::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::PaletteChange ||
        event->type() == QEvent::ApplicationPaletteChange ||
        event->type() == QEvent::StyleChange) {
        this->updateThemeIcons();
    }
}

// OSC Server

void MainWindow::openOSCSettings()
{
    // Set Local IP
    QString localIp = get_local_ip();
    if(!localIp.isEmpty()){
        oscConfigPicker->setInBoundIPv4(&localIp);
    }

    // Reset dialog result before showing so dismissal does not reuse a previous action.
    oscConfigPicker->ok = false;
    oscConfigPicker->Result = CasterOSCServerConfigPicker::Action_Neutral;

    // Run Setting Dialog
    oscConfigPicker->exec();
    if(oscConfigPicker->ok == true)
    {
        if(oscConfigPicker->Result == CasterOSCServerConfigPicker::Action_Start_Server){
            // Attempt To Start Server
            inbound_port = oscConfigPicker->getInboundPort();
            *outbound_ipv4 = oscConfigPicker->getOutboundIPv4();
            outbound_port = oscConfigPicker->getOutboundPort();
            if (socket->state() != QAbstractSocket::UnconnectedState) {
                socket->close();
            }
            bool bind_success = socket->bind(QHostAddress::Any, inbound_port);
            if(bind_success){
                // Notify User
                QMessageBox msgBox;
                msgBox.setText("Success!! OSC server running on port " + QString::number(inbound_port) + "\n");
                msgBox.setStandardButtons(QMessageBox::Close);
                msgBox.setDefaultButton(QMessageBox::Close);
                msgBox.setModal(true);
                msgBox.setWindowTitle("Connection Successful");
                msgBox.setWindowIcon(QIcon(":/res/img/about.png"));
                msgBox.exec();

                // Connect Listener Server
                connect(socket, SIGNAL(readyRead()), this, SLOT(executeOSCCommand()), Qt::UniqueConnection);

                //Update Diag
                oscConfigPicker->setIsServerRunning(true);

                // Reset Result
                oscConfigPicker->ok = false;
                oscConfigPicker->Result = CasterOSCServerConfigPicker::Action_Neutral;

                // Synchronize UI Info
                if (CasterBoard *board = currentBoard()) {
                    board->syncWithOSCClient();
                }
                this->syncWithOSCClient();
            } else {
                QMessageBox msgBox;
                msgBox.setText("Sorry, inbound port " + QString::number(inbound_port) + " appears to be in use.\nPlease try another port.\n");
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
    // OSC is transported via UDP. Limit allocations and drain queued datagrams.
    const qint64 maxPacketSize = 8192;
    while (socket->hasPendingDatagrams()) {
        const qint64 size = socket->pendingDatagramSize();
        if (size < 8 || size > maxPacketSize || (size % 4) != 0) {
            char discard;
            if (socket->readDatagram(&discard, 1) < 0)
                break; // Avoid spinning on a socket error; the datagram is otherwise discarded.
            qWarning() << "OSC: rejected packet size" << size;
            continue;
        }

        QByteArray datagram(static_cast<int>(size), 0);
        if (socket->readDatagram(datagram.data(), datagram.size()) != size) {
            qWarning() << "OSC: failed to read datagram";
            break;
        }

        // The application only handles direct OSC messages, not bundles.
        // Check the address and type-tag strings before invoking the parser.
        if (datagram.at(0) != '/')
            continue;
        const int addressEnd = datagram.indexOf('\0');
        if (addressEnd < 1)
            continue;
        const int tagsStart = (addressEnd + 4) & ~3;
        if (tagsStart >= datagram.size() || datagram.at(tagsStart) != ',' ||
            datagram.indexOf('\0', tagsStart) < 0) {
            qWarning() << "OSC: invalid address or type tags";
            continue;
        }

        try {
            OscReader reader(&datagram);
            std::unique_ptr<OscMessage> msg(reader.getMessage());
            if (!msg || msg->getNumValues() == 0)
                continue;

            QString address = msg->getAddress();
            QStringList address_params = address.split("/");
            QString values("");
            for (size_t i = 0; i < msg->getNumValues(); ++i) {
                if (i != 0) values += " | ";
                values += msg->getValue(i)->toString();
            }
            main_statusbar->showMessage("[OSC Message] Address: " + address +
                                        " Value(s): " + values, 500);

            if (address_params.value(1) == "castersoundboard")
                executeOneWayOSCCommand(msg.get(), address_params);
            else
                executeTwoWayOSCCommand(msg.get(), address_params);
        } catch (...) {
            // Never allow malformed network input to escape a Qt slot.
            qWarning() << "OSC: rejected malformed packet";
        }
    }
}

void MainWindow::executeOneWayOSCCommand(OscMessage *msg, QStringList address_params)
{
    if (!msg || msg->getNumValues() != 1)
        return;

    OscValue *val = msg->getValue(0);
    if (!val)
        return;

    try {
        if (address_params.value(2) == "board") {
            const QString boardName = address_params.value(3);
            if (boardName.isEmpty() || address_params.value(4) != "player")
                return;

            CasterBoard *board = findBoardByName(boardName);
            if (!board)
                return;

            const QString playerKey = address_params.value(5);
            if (playerKey.isEmpty() || !board->players->contains(playerKey) || address_params.value(6) != "modify")
                return;

            CasterPlayerWidget *player = board->players->value(playerKey);
            const QString prop = address_params.value(7);
            const QString subProp = address_params.value(8);

            if (prop == "volume" && subProp.isEmpty()) {
                player->volumeSlider->setValue(static_cast<int>(100 * val->toFloat()));
            } else if (prop == "track_position_percent" && subProp.isEmpty()) {
                player->trackBar->setValue(static_cast<int>(100 * val->toFloat()));
            } else if (prop == "play_state" && !subProp.isEmpty() && address_params.value(9).isEmpty()) {
                if (val->toInteger() != 1)
                    return;
                if (subProp == "play") {
                    player->playSound();
                } else if (subProp == "resume") {
                    player->resumeSound();
                } else if (subProp == "pause") {
                    player->pauseSound();
                } else if (subProp == "stop") {
                    player->stopSound();
                }
            } else if (prop == "loop_state" && subProp.isEmpty()) {
                player->setLoopState(val->toInteger());
            }
        } else if (address_params.value(2) == "global" && address_params.value(3) == "modify") {
            const QString prop = address_params.value(4);
            const QString subProp = address_params.value(5);

            if (prop == "all_play_states" && subProp == "stop" && address_params.value(6).isEmpty()) {
                if (val->toInteger() == 1)
                    this->stopAllSounds();
            } else if (prop == "audio_ducking_state" && subProp.isEmpty()) {
                setAudioDuckingState(val->toInteger(), false);
            }
        }
    } catch (...) {
        // Ignore type conversion errors on incoming OSC parameters.
    }
}

void MainWindow::executeTwoWayOSCCommand(OscMessage *msg, QStringList address_params)
{
    if (!msg || msg->getNumValues() != 1)
        return;

    OscValue *val = msg->getValue(0);
    if (!val)
        return;

    try {
        if (address_params.value(1) == "cbp") { // cbp = Current Board Player
            CasterBoard *board = currentBoard();
            if (!board)
                return;

            const QString playerKey = address_params.value(2);
            if (playerKey.isEmpty() || !board->players->contains(playerKey) || address_params.value(3) != "m")
                return;

            CasterPlayerWidget *player = board->players->value(playerKey);
            const QString prop = address_params.value(4);
            const QString subProp = address_params.value(5);

            if (prop == "vol" && subProp.isEmpty()) {
                player->volumeSlider->setValue(static_cast<int>(100 * val->toFloat()));
            } else if (prop == "t_p_p" && subProp.isEmpty()) {
                player->trackBar->setValue(static_cast<int>(100 * val->toFloat()));
            } else if (prop == "p_s" && !subProp.isEmpty() && address_params.value(6).isEmpty()) {
                if (val->toInteger() != 1)
                    return;
                if (subProp == "play_stop") {
                    player->play_stop_toggle();
                } else if (subProp == "resume_pause") {
                    player->resume_pause_toggle();
                }
            } else if (prop == "l_s" && subProp.isEmpty()) {
                player->setLoopState(val->toInteger());
            }
        } else if (address_params.value(1) == "glo") { // glo = global
            const QString action = address_params.value(2);
            if (action == "m") {
                const QString prop = address_params.value(3);
                const QString subProp = address_params.value(4);

                if (prop == "all_p_s" && subProp == "stop" && address_params.value(5).isEmpty()) {
                    if (val->toInteger() == 1)
                        this->stopAllSounds();
                } else if (prop == "audio_d_s" && subProp.isEmpty()) {
                    setAudioDuckingState(val->toInteger(), false);
                } else if (prop == "current_tab" && !subProp.isEmpty() && address_params.value(5).isEmpty()) {
                    if (val->toInteger() != 1)
                        return;
                    if (subProp == "next") {
                        this->switchToNextTab();
                    } else if (subProp == "prev") {
                        this->switchToPrevTab();
                    }
                }
            } else if (action == "sync") {
                if (val->toInteger() == 1) {
                    if (CasterBoard *board = currentBoard()) {
                        this->syncWithOSCClient();
                        board->syncWithOSCClient();
                    }
                }
            }
        }
    } catch (...) {
        // Ignore type conversion errors on incoming OSC parameters.
    }
}

void MainWindow::sendOSCMessageToClient(OscMessageComposer* message)
{
    std::unique_ptr<OscMessageComposer> ownedMessage(message);
    // Send OSC Message
    try{
        if(oscConfigPicker->getIsServerRunning()){
            std::unique_ptr<QByteArray> oscDatagram(message->getBytes());
            QHostAddress oscClientAddress(*outbound_ipv4);
            socket->writeDatagram(*oscDatagram, oscClientAddress, outbound_port);
        }
    } catch(...) { }

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
CasterBoard* MainWindow::findBoardByName(const QString &name) const
{
    for (int i = 0; i < mainTabContainer->count(); ++i) {
        if (auto *board = dynamic_cast<CasterBoard*>(mainTabContainer->widget(i))) {
            if (*board->soundBoardName == name)
                return board;
        }
    }
    return nullptr;
}

CasterBoard* MainWindow::currentBoard() const
{
    if (mainTabContainer->count() <= 0)
        return nullptr;
    return dynamic_cast<CasterBoard*>(mainTabContainer->currentWidget());
}

QString MainWindow::get_local_ip(){
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
             return address.toString();
    }
    return QString();
}

bool MainWindow::isDarkTheme() const
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (const QStyleHints *hints = QGuiApplication::styleHints()) {
        const Qt::ColorScheme scheme = hints->colorScheme();
        if (scheme == Qt::ColorScheme::Dark)
            return true;
        if (scheme == Qt::ColorScheme::Light)
            return false;
    }
#endif
    const QPalette pal = this->palette();
    return pal.color(QPalette::Window).lightness() < pal.color(QPalette::WindowText).lightness();
}

QIcon MainWindow::themedIcon(const QString &iconFileName) const
{
    const QString prefix = isDarkTheme() ? QStringLiteral(":/res/img/white/") : QStringLiteral(":/res/img/");
    return QIcon(prefix + iconFileName);
}

void MainWindow::updateThemeIcons()
{
    addNewTabButton->setIcon(themedIcon("newTab.png"));
    openTabButton->setIcon(themedIcon("open.png"));
    saveTabButton->setIcon(themedIcon("save.png"));
    saveAsTabButton->setIcon(themedIcon("save_as.png"));
    renameCurrentTabButton->setIcon(themedIcon("rename.png"));
    stopAllSoundsButton->setIcon(themedIcon("stopAll.png"));
    toggleAudioDuckingButton->setIcon(themedIcon(audio_duck_state == 0 ? "duck.png" : "unduck.png"));
    openSoundControlButton->setIcon(themedIcon("open_sound_control.png"));
    aboutButton->setIcon(themedIcon("about.png"));
}

void MainWindow::syncWithOSCClient(){
    //Update Audio Ducking
    OscMessageComposer* msg = writeOSCMessage("/glo/m/audio_d_s", audio_duck_state);
    this->sendOSCMessageToClient(msg);
}
