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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QWidget>

//forward declarations
class QObject;
class CasterBoard;
class QTabWidget;
class QPushButton;
class QToolBar;
class QStatusBar;
class QUdpSocket;
class QByteArray;
class OscMessage;
class CasterOSCServerConfigPicker;

class MainWindow : public QWidget //inherit from QWidget
{
    Q_OBJECT
public:
    //Constructor
    MainWindow(QWidget* parent = 0); //don't forget to pass the parent

    //PROPERTIES

    //METHODS

    //WIDGETS


protected:
    //PROPERTIES

    //METHODS
    void keyReleaseEvent(QKeyEvent *event);//Capture Hot Keys

    //WIDGETS

private:
    //PROPERTIES
    int audio_duck_state = 0;
    int previous_tab_index = 0;
    CasterOSCServerConfigPicker *oscConfigPicker;

    // OSC Server Settings
    int inbound_port = 5051;
    QString *outbound_ipv4;
    int outbound_port = 9000;

    //METHODS
    //OSC Composer Methods
    //Utility
    QString get_local_ip();

    //WIDGETS
    //Toolbar
    QToolBar *mainToolbar;
    // Push Buttons
    QPushButton *addNewTabButton;
    QPushButton *openTabButton;
    QPushButton *saveTabButton;
    QPushButton *saveAsTabButton;
    QPushButton *renameCurrentTabButton;
    QPushButton *stopAllSoundsButton;
    QPushButton *toggleAudioDuckingButton;
    QPushButton *openSoundControlButton;
    QPushButton *aboutButton;
    //Lower window area
    QTabWidget *mainTabContainer;
    // Bottom
    QStatusBar *main_statusbar;
    // UDP Server
    QUdpSocket *socket;


signals:
    //SIGNALS

public slots:
    //SLOTS
    void aboutBox();
    void addNewTab();
    void mainTabContainerTabClosedRequested(int tabIndex);
    void saveTab();
    void saveAsTab();
    void openProfile();
    void stopAllSounds();
    void renameCurrentTab();
    void toggleAudioDucking();
    // Tab Switching
    void switchToNextTab();
    void switchToPrevTab();
    void currentTabWasChanged(int tabIndex);
    void updateCurrentOSCTab(int tabIndex);
    // Board Signal Emissions Handlers
    void hotKeyExecution(QKeyEvent * event);
    void handleGlobalHotKeyEventFromCurrentWidget(QKeyEvent *event);
    //OSC Server

};

#endif // MAINWINDOW_H
