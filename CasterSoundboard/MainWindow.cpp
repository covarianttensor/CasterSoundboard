#include "actions/ActionEngine.h"
#include "actions/ActionsDialog.h"
#include <QSaveFile>
#include <QTabBar>
#include <QCloseEvent>
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
#include <QComboBox>
#include <QLabel>
#include <QMenu>
#include <QTabBar>
#include <QCheckBox>
#include <QDialog>
#include <QGroupBox>
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
#include <QSettings>
#include <QSignalBlocker>
#include <QMap>
#include <QtMultimedia/QMediaDevices>
#include <QtMultimedia/QAudioDevice>
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

    //~~Preferences Button~~
    settingsButton = new QPushButton;
    settingsButton->setIconSize(QSize(40,40));
    settingsButton->setToolTip("Preferences");

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
    auto actionsButton = new QPushButton;
    actionsButton->setIcon(QIcon(":/res/img/actions.png"));
    actionsButton->setIconSize(QSize(40,40));
    actionsButton->setToolTip(tr("Actions"));
    actionsButton->setAccessibleName(tr("Actions"));
    actionsButton->setObjectName("actionsButton");
    mainToolbar->addWidget(actionsButton);
    mainToolbar->addWidget(settingsButton);
    mainToolbar->addWidget(aboutButton);

    // Audio Output Selector
    mediaDevices = new QMediaDevices(this);
    audioOutputLabel = new QLabel(tr("Output:"));
    audioOutputLabel->setContentsMargins(8, 0, 4, 0);

    audioOutputComboBox = new QComboBox;
    audioOutputComboBox->setMinimumWidth(160);
    audioOutputComboBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    audioOutputComboBox->setToolTip(tr("Select audio output device for playback"));

    mainToolbar->addSeparator();
    mainToolbar->addWidget(audioOutputLabel);
    mainToolbar->addWidget(audioOutputComboBox);

    // Add toolbar to layout
    layout->addWidget(mainToolbar, 0, Qt::AlignLeft);

    //INIT MAIN TAB CONTAINER
    mainTabContainer = new QTabWidget;
    mainTabContainer->setTabsClosable(true);
    layout->addWidget(mainTabContainer, 1);

    connect(actionsButton, &QPushButton::clicked, this, [this] {
        auto board = qobject_cast<CasterBoard*>(mainTabContainer->currentWidget());
        if (!board) { showActionsError(tr("Open a tab before configuring Actions.")); return; }
        if (Actions::editProfiles(board, this, mainTabContainer)) refreshActions();
    });
    connect(Actions::Engine::instance(), &Actions::Engine::error, this, [this](QString error) {
        showActionsError(error);
    });
    refreshActions();

    // Status Bar
    main_statusbar = new QStatusBar;
    main_statusbar->setSizeGripEnabled(false);
    layout->addWidget(main_statusbar);

    //SET LAYOUT
    this->setLayout(layout);

    //MAKE CONNECTIONS
    connect(aboutButton,SIGNAL(clicked()),this,SLOT(aboutBox()));
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::openPreferences);
    connect(addNewTabButton,SIGNAL(clicked()),this,SLOT(addNewTab()));
    connect(mainTabContainer,SIGNAL(tabCloseRequested(int)),this,SLOT(mainTabContainerTabClosedRequested(int)));
    connect(mainTabContainer,SIGNAL(currentChanged(int)),this,SLOT(currentTabWasChanged(int)));
    mainTabContainer->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mainTabContainer->tabBar(), &QTabBar::customContextMenuRequested,
            this, &MainWindow::tabContextMenuRequested);
    connect(saveTabButton,SIGNAL(clicked()),this,SLOT(saveTab()));
    connect(saveAsTabButton,SIGNAL(clicked()),this,SLOT(saveAsTab()));
    connect(openTabButton,SIGNAL(clicked()),this,SLOT(openProfile()));
    connect(renameCurrentTabButton,SIGNAL(clicked()),this,SLOT(renameCurrentTab()));
    connect(stopAllSoundsButton,SIGNAL(clicked()),this,SLOT(stopAllSounds()));
    connect(toggleAudioDuckingButton,SIGNAL(clicked()),this,SLOT(toggleAudioDucking()));
    connect(openSoundControlButton, SIGNAL(clicked()), this, SLOT(openOSCSettings()) );
    populateAudioOutputs();
    connect(mediaDevices, &QMediaDevices::audioOutputsChanged, this, &MainWindow::populateAudioOutputs);
    connect(audioOutputComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onAudioOutputChanged);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (QGuiApplication::styleHints()) {
        connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
                this, [this](Qt::ColorScheme) { this->updateThemeIcons(); });
    }
#endif

    QSettings settings("CasterSoundboard", "CasterSoundboard");
    if (settings.value("osc/autoStart", false).toBool()) {
        startOSCServer();
    }

    // Properties

}

MainWindow::~MainWindow()
{
    delete outbound_ipv4;
}

//SLOTS
void MainWindow::mainTabContainerTabClosedRequested(int tabIndex)
{
    auto board = qobject_cast<CasterBoard*>(mainTabContainer->widget(tabIndex));
    if (board && board->hasAttachedActions()) {
        showActionsError(tr("Remove attached Actions before closing this tab.")); return;
    }
    if (board && board->actionsDirty) {
        auto answer = QMessageBox::question(this, "Unsaved Actions", "Save the Caster profile before closing?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (answer == QMessageBox::Cancel || (answer == QMessageBox::Save && !saveBoard(board, false))) return;
    }
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
    refreshActions();
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

void MainWindow::updateTabToolTips()
{
    QSettings settings("CasterSoundboard", "CasterSoundboard");
    const QString defaultProf = settings.value("profile/defaultProfile").toString();
    for (int i = 0; i < mainTabContainer->count(); ++i) {
        CasterBoard *board = qobject_cast<CasterBoard *>(mainTabContainer->widget(i));
        if (!board)
            continue;
        if (!board->profileFilePath->isEmpty()) {
            if (!defaultProf.isEmpty() && *board->profileFilePath == defaultProf) {
                mainTabContainer->setTabToolTip(i, QString("%1 [Default Startup Profile]").arg(*board->profileFilePath));
            } else {
                mainTabContainer->setTabToolTip(i, *board->profileFilePath);
            }
        } else {
            mainTabContainer->setTabToolTip(i, tr("Unsaved soundboard"));
        }
    }
}

void MainWindow::tabContextMenuRequested(const QPoint &pos)
{
    int tabIndex = mainTabContainer->tabBar()->tabAt(pos);
    if (tabIndex < 0)
        return;

    CasterBoard *board = qobject_cast<CasterBoard *>(mainTabContainer->widget(tabIndex));
    if (!board)
        return;

    QSettings settings("CasterSoundboard", "CasterSoundboard");
    const QString defaultProf = settings.value("profile/defaultProfile").toString();
    const bool isDefault = (!board->profileFilePath->isEmpty() && *board->profileFilePath == defaultProf);

    QMenu menu(this);
    if (isDefault) {
        menu.addAction(tr("Clear as Default Startup Profile"), this, [this]() {
            QSettings s("CasterSoundboard", "CasterSoundboard");
            s.remove("profile/defaultProfile");
            updateTabToolTips();
        });
    } else {
        menu.addAction(tr("Set as Default Startup Profile"), this, [this, board]() {
            if (board->profileFilePath->isEmpty()) {
                QMessageBox::information(this, tr("Default Profile"),
                    tr("Please save this soundboard tab to a .caster file before setting it as the default profile."));
                return;
            }
            QSettings s("CasterSoundboard", "CasterSoundboard");
            s.setValue("profile/defaultProfile", *board->profileFilePath);
            updateTabToolTips();
            QMessageBox::information(this, tr("Default Profile"),
                tr("'%1' set as default startup profile.").arg(*board->profileFilePath));
        });
    }
    menu.addSeparator();
    menu.addAction(tr("Rename Tab"), this, [this, tabIndex]() {
        mainTabContainer->setCurrentIndex(tabIndex);
        renameCurrentTab();
    });
    menu.addAction(tr("Close Tab"), this, [this, tabIndex]() {
        mainTabContainerTabClosedRequested(tabIndex);
    });
    menu.exec(mainTabContainer->tabBar()->mapToGlobal(pos));
}

void MainWindow::openPreferences()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Preferences"));
    dlg.setMinimumWidth(480);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dlg);

    // Section: Startup Profile
    QGroupBox *profileGroup = new QGroupBox(tr("Startup Soundboard Profile"), &dlg);
    QVBoxLayout *groupLayout = new QVBoxLayout(profileGroup);

    QSettings settings("CasterSoundboard", "CasterSoundboard");
    QString defaultProfile = settings.value("profile/defaultProfile").toString();

    QLabel *infoLabel = new QLabel(tr("Configure a soundboard profile to automatically load on startup (ideal for desktop autostart and headless Raspberry Pi / OSC setups):"), profileGroup);
    infoLabel->setWordWrap(true);
    groupLayout->addWidget(infoLabel);

    QLabel *currentPathLabel = new QLabel(defaultProfile.isEmpty() ? tr("None (starts with empty soundboard)") : defaultProfile, profileGroup);
    currentPathLabel->setWordWrap(true);
    currentPathLabel->setStyleSheet("font-weight: bold; padding: 6px; border: 1px solid palette(mid); background: palette(alternate-base);");
    groupLayout->addWidget(currentPathLabel);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *useCurrentBtn = new QPushButton(tr("Use Current Tab"), profileGroup);
    QPushButton *browseBtn = new QPushButton(tr("Browse..."), profileGroup);
    QPushButton *clearBtn = new QPushButton(tr("Clear Default"), profileGroup);
    btnLayout->addWidget(useCurrentBtn);
    btnLayout->addWidget(browseBtn);
    btnLayout->addWidget(clearBtn);
    groupLayout->addLayout(btnLayout);

    QCheckBox *reopenLastCheckbox = new QCheckBox(tr("Reopen last active profile on startup if no default profile is set"), profileGroup);
    reopenLastCheckbox->setChecked(settings.value("profile/reopenLastOnStartup", false).toBool());
    groupLayout->addWidget(reopenLastCheckbox);

    mainLayout->addWidget(profileGroup);

    // Section: OSC Server
    QGroupBox *oscGroup = new QGroupBox(tr("OSC Network Control"), &dlg);
    QVBoxLayout *oscLayout = new QVBoxLayout(oscGroup);
    QCheckBox *autoStartOscCheckbox = new QCheckBox(tr("Automatically start OSC server on application startup"), oscGroup);
    autoStartOscCheckbox->setChecked(settings.value("osc/autoStart", false).toBool());
    oscLayout->addWidget(autoStartOscCheckbox);
    mainLayout->addWidget(oscGroup);

    // Dialog buttons
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    QPushButton *closeBtn = new QPushButton(tr("Close"), &dlg);
    bottomLayout->addWidget(closeBtn);
    mainLayout->addLayout(bottomLayout);

    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    connect(useCurrentBtn, &QPushButton::clicked, this, [this, currentPathLabel]() {
        CasterBoard *board = currentBoard();
        if (!board) {
            QMessageBox::information(this, tr("Default Profile"), tr("Please create or open a soundboard tab first."));
            return;
        }
        if (board->profileFilePath->isEmpty()) {
            QMessageBox::information(this, tr("Default Profile"), tr("Please save this soundboard tab to a file before setting it as the default profile."));
            return;
        }
        QSettings s("CasterSoundboard", "CasterSoundboard");
        s.setValue("profile/defaultProfile", *board->profileFilePath);
        currentPathLabel->setText(*board->profileFilePath);
        updateTabToolTips();
    });

    connect(browseBtn, &QPushButton::clicked, this, [this, currentPathLabel]() {
        const QString path = QFileDialog::getOpenFileName(this, tr("Select Default Profile"), "", "Sound Board Profile (*.caster)");
        if (!path.isEmpty()) {
            QSettings s("CasterSoundboard", "CasterSoundboard");
            s.setValue("profile/defaultProfile", path);
            currentPathLabel->setText(path);
            updateTabToolTips();
        }
    });

    connect(clearBtn, &QPushButton::clicked, this, [this, currentPathLabel]() {
        QSettings s("CasterSoundboard", "CasterSoundboard");
        s.remove("profile/defaultProfile");
        currentPathLabel->setText(tr("None (starts with empty soundboard)"));
        updateTabToolTips();
    });

    connect(reopenLastCheckbox, &QCheckBox::toggled, this, [](bool checked) {
        QSettings s("CasterSoundboard", "CasterSoundboard");
        s.setValue("profile/reopenLastOnStartup", checked);
    });

    connect(autoStartOscCheckbox, &QCheckBox::toggled, this, [](bool checked) {
        QSettings s("CasterSoundboard", "CasterSoundboard");
        s.setValue("osc/autoStart", checked);
    });

    dlg.exec();
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
        updateTabToolTips();
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
    {
        QSettings settings("CasterSoundboard", "CasterSoundboard");
        settings.setValue("profile/lastOpenedProfile", filePath);
    }
    cb->reloadBoardFromPlayerStates();
    cb->setAllAudioDuckingStates(audio_duck_state);
    connect(cb, SIGNAL(globalHotKeyReleasedEvent(QKeyEvent*)),this,SLOT(handleGlobalHotKeyEventFromCurrentWidget(QKeyEvent*)));
    connect(cb, SIGNAL(_updateOSCClient(OscMessageComposer*)),this,SLOT(sendOSCMessageToClient(OscMessageComposer*)));
    const int index = mainTabContainer->addTab(cb, *cb->soundBoardName);
    mainTabContainer->setCurrentIndex(index);
    updateTabToolTips();
    return true;
}

bool MainWindow::saveBoard(CasterBoard *board, bool saveAs)
{
    if (!board) return false;
    QString path = *board->profileFilePath;
    if (saveAs || path.isEmpty()) {
        path = QFileDialog::getSaveFileName(this, "Save Caster Profile", path, "Caster Profile (*.caster)");
        if (path.isEmpty()) return false;
        if (!path.endsWith(".caster", Qt::CaseInsensitive)) path += ".caster";
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) { showProfileError(this, file.errorString()); return false; }
    QDataStream out(&file); out.setVersion(QDataStream::Qt_DefaultCompiledVersion);
    out << *board;
    if (out.status() != QDataStream::Ok) { file.cancelWriting(); showProfileError(this, "Cannot write Caster profile."); return false; }
    if (!file.commit()) { showProfileError(this, file.errorString()); return false; }
    *board->profileFilePath = path; board->actionsDirty = false; refreshActions();
    QSettings settings("CasterSoundboard", "CasterSoundboard");
    settings.setValue("profile/lastOpenedProfile", path);
    updateTabToolTips();
    return true;
}
void MainWindow::saveTab() { saveBoard(qobject_cast<CasterBoard*>(mainTabContainer->currentWidget()), false); }
void MainWindow::saveAsTab() { saveBoard(qobject_cast<CasterBoard*>(mainTabContainer->currentWidget()), true); }

void MainWindow::showActionsError(const QString &error)
{
    auto message = new QMessageBox(QMessageBox::Warning, tr("Actions"), error,
                                  QMessageBox::Ok, this);
    message->setTextFormat(Qt::PlainText);
    message->setAttribute(Qt::WA_DeleteOnClose);
    message->setModal(false);
    message->show();
}

void MainWindow::refreshActions()
{
    for (int i = 0; i < mainTabContainer->count(); ++i) {
        auto board = qobject_cast<CasterBoard*>(mainTabContainer->widget(i));
        if (!board) continue;
        const bool attached = board->hasAttachedActions();
        for (auto side : {QTabBar::LeftSide, QTabBar::RightSide}) {
            if (auto button = mainTabContainer->tabBar()->tabButton(i, side)) {
                button->setEnabled(!attached);
                button->setToolTip(attached ? "Remove attached Actions before closing this tab." : "Close tab");
            }
        }
    }
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    for (int i = 0; i < mainTabContainer->count(); ++i) {
        auto board = qobject_cast<CasterBoard*>(mainTabContainer->widget(i));
        if (!board || !board->actionsDirty) continue;
        auto answer = QMessageBox::question(this, "Unsaved Actions", "Save changes to " + *board->soundBoardName + "?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (answer == QMessageBox::Cancel || (answer == QMessageBox::Save && !saveBoard(board, false))) { event->ignore(); return; }
    }
    Actions::Engine::instance()->shutdown();
    event->accept();
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
    Actions::Engine::instance()->shutdown();
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

void MainWindow::populateAudioOutputs()
{
    if (!audioOutputComboBox)
        return;

    QSignalBlocker blocker(audioOutputComboBox);

    // Prefer currently selected ID if combobox was already populated,
    // otherwise fallback to saved QSettings
    QByteArray activeId = audioOutputComboBox->currentData().toByteArray();
    if (activeId.isEmpty()) {
        QSettings settings("CasterSoundboard", "CasterSoundboard");
        activeId = settings.value("audio/outputDeviceId").toByteArray();
    }

    audioOutputComboBox->clear();
    audioOutputComboBox->addItem(tr("Default System Output"), QByteArray());

    const QList<QAudioDevice> devices = QMediaDevices::audioOutputs();

    // Disambiguate duplicate descriptions if any exist
    QMap<QString, int> descCounts;
    for (const QAudioDevice &dev : devices) {
        descCounts[dev.description()]++;
    }
    QMap<QString, int> descIndex;

    int targetIndex = 0;
    for (const QAudioDevice &dev : devices) {
        QString label = dev.description();
        if (descCounts[dev.description()] > 1) {
            descIndex[dev.description()]++;
            label += QString(" (%1)").arg(descIndex[dev.description()]);
        }
        if (dev.isDefault()) {
            label += tr(" (Default)");
        }
        audioOutputComboBox->addItem(label, dev.id());
        if (!activeId.isEmpty() && dev.id() == activeId) {
            targetIndex = audioOutputComboBox->count() - 1;
        }
    }

    audioOutputComboBox->setCurrentIndex(targetIndex);

    // Apply the resolved device to all players
    QByteArray chosenId = audioOutputComboBox->currentData().toByteArray();
    QAudioDevice chosenDevice;
    if (!chosenId.isEmpty()) {
        for (const QAudioDevice &dev : devices) {
            if (dev.id() == chosenId) {
                chosenDevice = dev;
                break;
            }
        }
    }
    CasterMediaPlayer::setGlobalAudioDevice(chosenDevice);
}

void MainWindow::onAudioOutputChanged(int index)
{
    Q_UNUSED(index);
    if (!audioOutputComboBox)
        return;

    const QByteArray selectedId = audioOutputComboBox->currentData().toByteArray();
    QSettings settings("CasterSoundboard", "CasterSoundboard");
    settings.setValue("audio/outputDeviceId", selectedId);

    QAudioDevice chosenDevice;
    if (!selectedId.isEmpty()) {
        const QList<QAudioDevice> devices = QMediaDevices::audioOutputs();
        for (const QAudioDevice &dev : devices) {
            if (dev.id() == selectedId) {
                chosenDevice = dev;
                break;
            }
        }
    }
    CasterMediaPlayer::setGlobalAudioDevice(chosenDevice);
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
            bool bind_success = startOSCServer(oscConfigPicker->getInboundPort(),
                                               oscConfigPicker->getOutboundIPv4(),
                                               oscConfigPicker->getOutboundPort());
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

                // Reset Result
                oscConfigPicker->ok = false;
                oscConfigPicker->Result = CasterOSCServerConfigPicker::Action_Neutral;
            } else {
                QMessageBox msgBox;
                msgBox.setText("Sorry, inbound port " + QString::number(inbound_port) + " appears to be in use.\nPlease try another port.\n");
                msgBox.setStandardButtons(QMessageBox::Close);
                msgBox.setDefaultButton(QMessageBox::Close);
                msgBox.setModal(true);
                msgBox.setWindowTitle("Port Connection Error");
                msgBox.setWindowIcon(QIcon(":/res/img/about.png"));
                msgBox.exec();

                // Reset Result
                oscConfigPicker->ok = false;
                oscConfigPicker->Result = CasterOSCServerConfigPicker::Action_Neutral;
            }

        } else if (oscConfigPicker->Result == CasterOSCServerConfigPicker::Action_Kill_Server ){
            stopOSCServer();

            // Reset Result
            oscConfigPicker->ok = false;
            oscConfigPicker->Result = CasterOSCServerConfigPicker::Action_Neutral;
        }

    }
}

bool MainWindow::startOSCServer(int inboundPort, const QString &outboundIp, int outboundPort)
{
    if (inboundPort > 0)
        inbound_port = inboundPort;
    if (!outboundIp.isEmpty())
        *outbound_ipv4 = outboundIp;
    if (outboundPort > 0)
        outbound_port = outboundPort;

    if (socket->state() != QAbstractSocket::UnconnectedState) {
        socket->close();
    }
    bool bind_success = socket->bind(QHostAddress::Any, inbound_port);
    if (bind_success) {
        connect(socket, &QUdpSocket::readyRead, this, &MainWindow::executeOSCCommand, Qt::UniqueConnection);
        if (oscConfigPicker) {
            oscConfigPicker->setIsServerRunning(true);
            oscConfigPicker->setInBoundPort(inbound_port);
            oscConfigPicker->setOutBoundIPv4(outbound_ipv4);
            oscConfigPicker->setOutBoundPort(outbound_port);
        }
        if (CasterBoard *board = currentBoard()) {
            board->syncWithOSCClient();
        }
        this->syncWithOSCClient();
        return true;
    } else {
        if (oscConfigPicker) {
            oscConfigPicker->setIsServerRunning(false);
        }
        return false;
    }
}

void MainWindow::stopOSCServer()
{
    disconnect(socket, &QUdpSocket::readyRead, this, &MainWindow::executeOSCCommand);
    if (socket->state() != QAbstractSocket::UnconnectedState) {
        socket->close();
    }
    if (oscConfigPicker) {
        oscConfigPicker->setIsServerRunning(false);
    }
}

bool MainWindow::isOSCServerRunning() const
{
    return socket && (socket->state() == QAbstractSocket::BoundState);
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
    settingsButton->setIcon(themedIcon("cog.png"));
    aboutButton->setIcon(themedIcon("about.png"));
}

void MainWindow::syncWithOSCClient(){
    //Update Audio Ducking
    OscMessageComposer* msg = writeOSCMessage("/glo/m/audio_d_s", audio_duck_state);
    this->sendOSCMessageToClient(msg);
}
