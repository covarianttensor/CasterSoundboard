/*
 * Copyright 2017 aszlig
 *
 * This file is part of CasterSoundboard. An application for playing hot-keyed
 * sounds. For more information, please visit:
 *
 * https://github.com/JupiterBroadcasting/CasterSoundboard
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
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "CasterBoardLayout.h"

CasterBoardLayout::CasterBoardLayout(const QString &letter, int posY, int posX)
{
    this->letter = letter;
    this->posY = posY;
    this->posX = posX;
}
