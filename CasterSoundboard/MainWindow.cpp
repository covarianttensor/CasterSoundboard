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
#include <QMessageBox>
#include <QInputDialog>
#include <QToolBar>
#include <QPushButton>
#include <QTableWidget>
#include <QGridLayout>
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QFileDialog>

//CONSTRUCTOR
MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
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

    //~~Rename Tab~~
    renameCurrentTabButton = new QPushButton;
    renameCurrentTabButton->setIcon(QIcon(":/res/img/rename.png"));
    renameCurrentTabButton->setIconSize(QSize(40,40));

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
    mainToolbar->addWidget(aboutButton);
    // Add toolbar to layout
    layout->addWidget(mainToolbar, 0, 0, Qt::AlignLeft);

    //INIT MAIN TAB CONTAINER
    mainTabContainer = new QTabWidget;
    mainTabContainer->setTabsClosable(true);
    layout->addWidget(mainTabContainer,1,0, 2, 0);

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
            out.setVersion(QDataStream::Qt_5_8);
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
                out.setVersion(QDataStream::Qt_5_8);
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
            out.setVersion(QDataStream::Qt_5_8);
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
        in.setVersion(QDataStream::Qt_5_8);
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
    for(int i = 0; i < mainTabContainer->count(); i++)
    {
        dynamic_cast<CasterBoard*>(mainTabContainer->widget(i))->stopAllSounds();
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
