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
#include <QMessageBox>
#include <QInputDialog>
#include <QPushButton>
#include <QTableWidget>
#include <QGridLayout>

//CONSTRUCTOR
MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    //SET WINDOW PROPETIES
    this->setWindowTitle("CasterSoundboard");
    this->setWindowIcon(QIcon(":/res/img/app.png"));

    //DENFINE LAYOUT
    QGridLayout *layout = new QGridLayout;

    //INIT NEW TAB PUSH BUTTONS
    aboutButton = new QPushButton;
    aboutButton->setIcon(QIcon(":/res/img/about.png"));
    aboutButton->setIconSize(QSize(25,25));
    aboutButton->setFixedSize(30,30);
    layout->addWidget(aboutButton,0,0, Qt::AlignLeft);

    addNewTabButton = new QPushButton;
    addNewTabButton->setIcon(QIcon(":/res/img/newTab.png"));
    addNewTabButton->setIconSize(QSize(25,25));
    addNewTabButton->setFixedSize(30,30);
    layout->addWidget(addNewTabButton,0,1, Qt::AlignRight);

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

}

//SLOTS
void MainWindow::aboutBox()
{
    QMessageBox msgBox;
    msgBox.setText("CasterSoundboard (v0.1)\nAuthor: Oscar Cerna\ne-mail: covarianttensor@gmail.com\nLicense: LGPL v3\n© 2013 Oscar Cerna");
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
