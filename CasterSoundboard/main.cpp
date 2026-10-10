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
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFileInfo>
#include <QDebug>


int main(int argc, char *argv[])
{
    //START APPLICATION
    QApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("CasterSoundboard");
    parser.addHelpOption();
    QCommandLineOption profileOption(QStringList() << "p" << "profile",
                                     "Load a .caster profile at startup", "file");
    parser.addOption(profileOption);
    parser.addPositionalArgument("profile", "Optional .caster profile file", "[profile]");
    parser.process(a);

    const QStringList positional = parser.positionalArguments();
    if (positional.size() > 1 || (parser.isSet(profileOption) && !positional.isEmpty())) {
        qCritical() << "Specify only one profile, either as --profile FILE or FILE.";
        return 2;
    }
    const QString profile = parser.isSet(profileOption) ? parser.value(profileOption)
                                                        : positional.value(0);
    if ((parser.isSet(profileOption) || !positional.isEmpty()) && profile.isEmpty()) {
        qCritical() << "Profile filename must not be empty.";
        return 2;
    }
    //CREATE MAIN WINDOW
    MainWindow *w = new MainWindow;
    //SET MAIN WINDOW SIZE
    w->resize(1000, 500);
    //SHOW MAIN WINDOW
    w->show();
    if (!profile.isEmpty()) {
        const QString absolutePath = QFileInfo(profile).absoluteFilePath();
        QString error;
        if (!w->loadProfile(absolutePath, error)) {
            qCritical().noquote() << error;
            delete w;
            return 1;
        }
    }
    //END APPLICATION
    return a.exec();
}
