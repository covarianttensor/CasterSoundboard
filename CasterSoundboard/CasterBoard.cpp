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
#include "CasterBoard.h"
#include "CasterPlayer.h"
#include <QGridLayout>

//Constructor=============================================
CasterBoard::CasterBoard(QWidget* parent) : QWidget(parent)
{
    QGridLayout *boardLayout = new QGridLayout(this);

    //WIDGETS
    player1 = new CasterPlayerWidget;
    player1->setHotKeyLetter("1");
    player2 = new CasterPlayerWidget;
    player2->setHotKeyLetter("2");
    player3 = new CasterPlayerWidget;
    player3->setHotKeyLetter("3");
    player4 = new CasterPlayerWidget;
    player4->setHotKeyLetter("4");
    player5 = new CasterPlayerWidget;
    player5->setHotKeyLetter("5");
    player6 = new CasterPlayerWidget;
    player6->setHotKeyLetter("6");
    player7 = new CasterPlayerWidget;
    player7->setHotKeyLetter("7");
    player8 = new CasterPlayerWidget;
    player8->setHotKeyLetter("8");

    playerQ = new CasterPlayerWidget;
    playerQ->setHotKeyLetter("Q");
    playerW = new CasterPlayerWidget;
    playerW->setHotKeyLetter("W");
    playerE = new CasterPlayerWidget;
    playerE->setHotKeyLetter("E");
    playerR = new CasterPlayerWidget;
    playerR->setHotKeyLetter("R");
    playerT = new CasterPlayerWidget;
    playerT->setHotKeyLetter("T");
    playerY = new CasterPlayerWidget;
    playerY->setHotKeyLetter("Y");
    playerU = new CasterPlayerWidget;
    playerU->setHotKeyLetter("U");
    playerI = new CasterPlayerWidget;
    playerI->setHotKeyLetter("I");

    playerA = new CasterPlayerWidget;
    playerA->setHotKeyLetter("A");
    playerS = new CasterPlayerWidget;
    playerS->setHotKeyLetter("S");
    playerD = new CasterPlayerWidget;
    playerD->setHotKeyLetter("D");
    playerF = new CasterPlayerWidget;
    playerF->setHotKeyLetter("F");
    playerG = new CasterPlayerWidget;
    playerG->setHotKeyLetter("G");
    playerH = new CasterPlayerWidget;
    playerH->setHotKeyLetter("H");
    playerJ = new CasterPlayerWidget;
    playerJ->setHotKeyLetter("J");
    playerK = new CasterPlayerWidget;
    playerK->setHotKeyLetter("K");

    playerZ = new CasterPlayerWidget;
    playerZ->setHotKeyLetter("Z");
    playerX = new CasterPlayerWidget;
    playerX->setHotKeyLetter("X");
    playerC = new CasterPlayerWidget;
    playerC->setHotKeyLetter("C");
    playerV = new CasterPlayerWidget;
    playerV->setHotKeyLetter("V");
    playerB = new CasterPlayerWidget;
    playerB->setHotKeyLetter("B");
    playerN = new CasterPlayerWidget;
    playerN->setHotKeyLetter("N");
    playerM = new CasterPlayerWidget;
    playerM->setHotKeyLetter("M");
    playerCOMMA = new CasterPlayerWidget;
    playerCOMMA->setHotKeyLetter(",");

    //ADD TO LAYOUT
    boardLayout->addWidget(player1, 0,0);
    boardLayout->addWidget(player2, 0,1);
    boardLayout->addWidget(player3, 0,2);
    boardLayout->addWidget(player4, 0,3);
    boardLayout->addWidget(player5, 0,4);
    boardLayout->addWidget(player6, 0,5);
    boardLayout->addWidget(player7, 0,6);
    boardLayout->addWidget(player8, 0,7);

    boardLayout->addWidget(playerQ, 1,0);
    boardLayout->addWidget(playerW, 1,1);
    boardLayout->addWidget(playerE, 1,2);
    boardLayout->addWidget(playerR, 1,3);
    boardLayout->addWidget(playerT, 1,4);
    boardLayout->addWidget(playerY, 1,5);
    boardLayout->addWidget(playerU, 1,6);
    boardLayout->addWidget(playerI, 1,7);

    boardLayout->addWidget(playerA, 2,0);
    boardLayout->addWidget(playerS, 2,1);
    boardLayout->addWidget(playerD, 2,2);
    boardLayout->addWidget(playerF, 2,3);
    boardLayout->addWidget(playerG, 2,4);
    boardLayout->addWidget(playerH, 2,5);
    boardLayout->addWidget(playerJ, 2,6);
    boardLayout->addWidget(playerK, 2,7);

    boardLayout->addWidget(playerZ, 3,0);
    boardLayout->addWidget(playerX, 3,1);
    boardLayout->addWidget(playerC, 3,2);
    boardLayout->addWidget(playerV, 3,3);
    boardLayout->addWidget(playerB, 3,4);
    boardLayout->addWidget(playerN, 3,5);
    boardLayout->addWidget(playerM, 3,6);
    boardLayout->addWidget(playerCOMMA, 3,7);
}

void CasterBoard::keyReleaseEvent(QKeyEvent *event)
{
    //Handles All Hot Key Behavior
    //BOARD 1
    if(event->key() == Qt::Key_1)
    {
        if(player1->player->state() == QMediaPlayer::PlayingState)
        {
            player1->stopSound();
        }
        else if (player1->player->state() == QMediaPlayer::PausedState)
        {
            player1->playSound();
        }
        else if (player1->player->state() == QMediaPlayer::StoppedState)
        {
            player1->playSound();
        }
    }
    else if(event->key() == Qt::Key_2)
    {
        if(player2->player->state() == QMediaPlayer::PlayingState)
        {
            player2->stopSound();
        }
        else if (player2->player->state() == QMediaPlayer::PausedState)
        {
            player2->playSound();
        }
        else if (player2->player->state() == QMediaPlayer::StoppedState)
        {
            player2->playSound();
        }
    }
    else if(event->key() == Qt::Key_3)
    {
        if(player3->player->state() == QMediaPlayer::PlayingState)
        {
            player3->stopSound();
        }
        else if (player3->player->state() == QMediaPlayer::PausedState)
        {
            player3->playSound();
        }
        else if (player3->player->state() == QMediaPlayer::StoppedState)
        {
            player3->playSound();
        }
    }
    else if(event->key() == Qt::Key_4)
    {
        if(player4->player->state() == QMediaPlayer::PlayingState)
        {
            player4->stopSound();
        }
        else if (player4->player->state() == QMediaPlayer::PausedState)
        {
            player4->playSound();
        }
        else if (player4->player->state() == QMediaPlayer::StoppedState)
        {
            player4->playSound();
        }
    }
    else if(event->key() == Qt::Key_5)
    {
        if(player5->player->state() == QMediaPlayer::PlayingState)
        {
            player5->stopSound();
        }
        else if (player5->player->state() == QMediaPlayer::PausedState)
        {
            player5->playSound();
        }
        else if (player5->player->state() == QMediaPlayer::StoppedState)
        {
            player5->playSound();
        }
    }
    else if(event->key() == Qt::Key_6)
    {
        if(player6->player->state() == QMediaPlayer::PlayingState)
        {
            player6->stopSound();
        }
        else if (player6->player->state() == QMediaPlayer::PausedState)
        {
            player6->playSound();
        }
        else if (player6->player->state() == QMediaPlayer::StoppedState)
        {
            player6->playSound();
        }
    }
    else if(event->key() == Qt::Key_7)
    {
        if(player7->player->state() == QMediaPlayer::PlayingState)
        {
            player7->stopSound();
        }
        else if (player7->player->state() == QMediaPlayer::PausedState)
        {
            player7->playSound();
        }
        else if (player7->player->state() == QMediaPlayer::StoppedState)
        {
            player7->playSound();
        }
    }
    else if(event->key() == Qt::Key_8)
    {
        if(player8->player->state() == QMediaPlayer::PlayingState)
        {
            player8->stopSound();
        }
        else if (player8->player->state() == QMediaPlayer::PausedState)
        {
            player8->playSound();
        }
        else if (player8->player->state() == QMediaPlayer::StoppedState)
        {
            player8->playSound();
        }
    }
    //BOARD 2
    else if(event->key() == Qt::Key_Q)
    {
        if(playerQ->player->state() == QMediaPlayer::PlayingState)
        {
            playerQ->stopSound();
        }
        else if (playerQ->player->state() == QMediaPlayer::PausedState)
        {
            playerQ->playSound();
        }
        else if (playerQ->player->state() == QMediaPlayer::StoppedState)
        {
            playerQ->playSound();
        }
    }
    else if(event->key() == Qt::Key_W)
    {
        if(playerW->player->state() == QMediaPlayer::PlayingState)
        {
            playerW->stopSound();
        }
        else if (playerW->player->state() == QMediaPlayer::PausedState)
        {
            playerW->playSound();
        }
        else if (playerW->player->state() == QMediaPlayer::StoppedState)
        {
            playerW->playSound();
        }
    }
    else if(event->key() == Qt::Key_E)
    {
        if(playerE->player->state() == QMediaPlayer::PlayingState)
        {
            playerE->stopSound();
        }
        else if (playerE->player->state() == QMediaPlayer::PausedState)
        {
            playerE->playSound();
        }
        else if (playerE->player->state() == QMediaPlayer::StoppedState)
        {
            playerE->playSound();
        }
    }
    else if(event->key() == Qt::Key_R)
    {
        if(playerR->player->state() == QMediaPlayer::PlayingState)
        {
            playerR->stopSound();
        }
        else if (playerR->player->state() == QMediaPlayer::PausedState)
        {
            playerR->playSound();
        }
        else if (playerR->player->state() == QMediaPlayer::StoppedState)
        {
            playerR->playSound();
        }
    }
    else if(event->key() == Qt::Key_T)
    {
        if(playerT->player->state() == QMediaPlayer::PlayingState)
        {
            playerT->stopSound();
        }
        else if (playerT->player->state() == QMediaPlayer::PausedState)
        {
            playerT->playSound();
        }
        else if (playerT->player->state() == QMediaPlayer::StoppedState)
        {
            playerT->playSound();
        }
    }
    else if(event->key() == Qt::Key_Y)
    {
        if(playerY->player->state() == QMediaPlayer::PlayingState)
        {
            playerY->stopSound();
        }
        else if (playerY->player->state() == QMediaPlayer::PausedState)
        {
            playerY->playSound();
        }
        else if (playerY->player->state() == QMediaPlayer::StoppedState)
        {
            playerY->playSound();
        }
    }
    else if(event->key() == Qt::Key_U)
    {
        if(playerU->player->state() == QMediaPlayer::PlayingState)
        {
            playerU->stopSound();
        }
        else if (playerU->player->state() == QMediaPlayer::PausedState)
        {
            playerU->playSound();
        }
        else if (playerU->player->state() == QMediaPlayer::StoppedState)
        {
            playerU->playSound();
        }
    }
    else if(event->key() == Qt::Key_I)
    {
        if(playerI->player->state() == QMediaPlayer::PlayingState)
        {
            playerI->stopSound();
        }
        else if (playerI->player->state() == QMediaPlayer::PausedState)
        {
            playerI->playSound();
        }
        else if (playerI->player->state() == QMediaPlayer::StoppedState)
        {
            playerI->playSound();
        }
    }
    //BOARD 3
    else if(event->key() == Qt::Key_A)
    {
        if(playerA->player->state() == QMediaPlayer::PlayingState)
        {
            playerA->stopSound();
        }
        else if (playerA->player->state() == QMediaPlayer::PausedState)
        {
            playerA->playSound();
        }
        else if (playerA->player->state() == QMediaPlayer::StoppedState)
        {
            playerA->playSound();
        }
    }
    else if(event->key() == Qt::Key_S)
    {
        if(playerS->player->state() == QMediaPlayer::PlayingState)
        {
            playerS->stopSound();
        }
        else if (playerS->player->state() == QMediaPlayer::PausedState)
        {
            playerS->playSound();
        }
        else if (playerS->player->state() == QMediaPlayer::StoppedState)
        {
            playerS->playSound();
        }
    }
    else if(event->key() == Qt::Key_D)
    {
        if(playerD->player->state() == QMediaPlayer::PlayingState)
        {
            playerD->stopSound();
        }
        else if (playerD->player->state() == QMediaPlayer::PausedState)
        {
            playerD->playSound();
        }
        else if (playerD->player->state() == QMediaPlayer::StoppedState)
        {
            playerD->playSound();
        }
    }
    else if(event->key() == Qt::Key_F)
    {
        if(playerF->player->state() == QMediaPlayer::PlayingState)
        {
            playerF->stopSound();
        }
        else if (playerF->player->state() == QMediaPlayer::PausedState)
        {
            playerF->playSound();
        }
        else if (playerF->player->state() == QMediaPlayer::StoppedState)
        {
            playerF->playSound();
        }
    }
    else if(event->key() == Qt::Key_G)
    {
        if(playerG->player->state() == QMediaPlayer::PlayingState)
        {
            playerG->stopSound();
        }
        else if (playerG->player->state() == QMediaPlayer::PausedState)
        {
            playerG->playSound();
        }
        else if (playerG->player->state() == QMediaPlayer::StoppedState)
        {
            playerG->playSound();
        }
    }
    else if(event->key() == Qt::Key_H)
    {
        if(playerH->player->state() == QMediaPlayer::PlayingState)
        {
            playerH->stopSound();
        }
        else if (playerH->player->state() == QMediaPlayer::PausedState)
        {
            playerH->playSound();
        }
        else if (playerH->player->state() == QMediaPlayer::StoppedState)
        {
            playerH->playSound();
        }
    }
    else if(event->key() == Qt::Key_J)
    {
        if(playerJ->player->state() == QMediaPlayer::PlayingState)
        {
            playerJ->stopSound();
        }
        else if (playerJ->player->state() == QMediaPlayer::PausedState)
        {
            playerJ->playSound();
        }
        else if (playerJ->player->state() == QMediaPlayer::StoppedState)
        {
            playerJ->playSound();
        }
    }
    else if(event->key() == Qt::Key_K)
    {
        if(playerK->player->state() == QMediaPlayer::PlayingState)
        {
            playerK->stopSound();
        }
        else if (playerK->player->state() == QMediaPlayer::PausedState)
        {
            playerK->playSound();
        }
        else if (playerK->player->state() == QMediaPlayer::StoppedState)
        {
            playerK->playSound();
        }
    }
    //BOARD 4
    else if(event->key() == Qt::Key_Z)
    {
        if(playerZ->player->state() == QMediaPlayer::PlayingState)
        {
            playerZ->stopSound();
        }
        else if (playerZ->player->state() == QMediaPlayer::PausedState)
        {
            playerZ->playSound();
        }
        else if (playerZ->player->state() == QMediaPlayer::StoppedState)
        {
            playerZ->playSound();
        }
    }
    else if(event->key() == Qt::Key_X)
    {
        if(playerX->player->state() == QMediaPlayer::PlayingState)
        {
            playerX->stopSound();
        }
        else if (playerX->player->state() == QMediaPlayer::PausedState)
        {
            playerX->playSound();
        }
        else if (playerX->player->state() == QMediaPlayer::StoppedState)
        {
            playerX->playSound();
        }
    }
    else if(event->key() == Qt::Key_C)
    {
        if(playerC->player->state() == QMediaPlayer::PlayingState)
        {
            playerC->stopSound();
        }
        else if (playerC->player->state() == QMediaPlayer::PausedState)
        {
            playerC->playSound();
        }
        else if (playerC->player->state() == QMediaPlayer::StoppedState)
        {
            playerC->playSound();
        }
    }
    else if(event->key() == Qt::Key_V)
    {
        if(playerV->player->state() == QMediaPlayer::PlayingState)
        {
            playerV->stopSound();
        }
        else if (playerV->player->state() == QMediaPlayer::PausedState)
        {
            playerV->playSound();
        }
        else if (playerV->player->state() == QMediaPlayer::StoppedState)
        {
            playerV->playSound();
        }
    }
    else if(event->key() == Qt::Key_B)
    {
        if(playerB->player->state() == QMediaPlayer::PlayingState)
        {
            playerB->stopSound();
        }
        else if (playerB->player->state() == QMediaPlayer::PausedState)
        {
            playerB->playSound();
        }
        else if (playerB->player->state() == QMediaPlayer::StoppedState)
        {
            playerB->playSound();
        }
    }
    else if(event->key() == Qt::Key_N)
    {
        if(playerN->player->state() == QMediaPlayer::PlayingState)
        {
            playerN->stopSound();
        }
        else if (playerN->player->state() == QMediaPlayer::PausedState)
        {
            playerN->playSound();
        }
        else if (playerN->player->state() == QMediaPlayer::StoppedState)
        {
            playerN->playSound();
        }
    }
    else if(event->key() == Qt::Key_M)
    {
        if(playerM->player->state() == QMediaPlayer::PlayingState)
        {
            playerM->stopSound();
        }
        else if (playerM->player->state() == QMediaPlayer::PausedState)
        {
            playerM->playSound();
        }
        else if (playerM->player->state() == QMediaPlayer::StoppedState)
        {
            playerM->playSound();
        }
    }
    else if(event->key() == Qt::Key_Comma)
    {
        if(playerCOMMA->player->state() == QMediaPlayer::PlayingState)
        {
            playerCOMMA->stopSound();
        }
        else if (playerCOMMA->player->state() == QMediaPlayer::PausedState)
        {
            playerCOMMA->playSound();
        }
        else if (playerCOMMA->player->state() == QMediaPlayer::StoppedState)
        {
            playerCOMMA->playSound();
        }
    }
}
