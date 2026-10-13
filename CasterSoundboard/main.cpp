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
#include <QSettings>
#include <QDebug>
#include <cstring>


int main(int argc, char *argv[])
{
    // Default to FFmpeg backend for Qt 6 Multimedia on Linux across all distributions
    if (qEnvironmentVariableIsEmpty("QT_MEDIA_BACKEND")) {
        qputenv("QT_MEDIA_BACKEND", "ffmpeg");
    }

    // Check for headless mode or absent display environment before initializing QApplication
    bool headless = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--headless") == 0) {
            headless = true;
            break;
        }
    }
    const bool noDisplay = qEnvironmentVariableIsEmpty("DISPLAY") && qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY");
    if ((headless || noDisplay) && qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
        headless = true;
    }

    //START APPLICATION
    QApplication a(argc, argv);
    a.setOrganizationName("CasterSoundboard");
    a.setApplicationName("CasterSoundboard");

    QCommandLineParser parser;
    parser.setApplicationDescription("CasterSoundboard");
    parser.addHelpOption();

    QCommandLineOption headlessOption("headless",
                                      "Run in headless mode without showing the GUI window (ideal for Raspberry Pi/server OSC setups)");
    parser.addOption(headlessOption);

    QCommandLineOption profileOption(QStringList() << "p" << "profile",
                                     "Load a .caster profile at startup", "file");
    parser.addOption(profileOption);

    QCommandLineOption setDefaultProfileOption("set-default-profile",
                                               "Set the specified .caster profile as the default startup profile in settings and exit", "file");
    parser.addOption(setDefaultProfileOption);

    QCommandLineOption clearDefaultProfileOption("clear-default-profile",
                                                 "Clear the configured default startup profile from settings and exit");
    parser.addOption(clearDefaultProfileOption);

    QCommandLineOption showDefaultProfileOption("show-default-profile",
                                                "Show the currently configured default startup profile and exit");
    parser.addOption(showDefaultProfileOption);

    QCommandLineOption oscPortOption("osc-port",
                                     "Inbound UDP port for OSC commands (defaults to 5051)", "port");
    parser.addOption(oscPortOption);

    QCommandLineOption autoStartOscOption("auto-start-osc",
                                          "Start the OSC server automatically on launch");
    parser.addOption(autoStartOscOption);

    parser.addPositionalArgument("profile", "Optional .caster profile file", "[profile]");
    parser.process(a);

    QSettings settings("CasterSoundboard", "CasterSoundboard");

    if (parser.isSet(showDefaultProfileOption)) {
        const QString def = settings.value("profile/defaultProfile").toString();
        if (def.isEmpty()) {
            qInfo() << "No default profile configured.";
        } else {
            qInfo().noquote() << def;
        }
        return 0;
    }

    if (parser.isSet(clearDefaultProfileOption)) {
        settings.remove("profile/defaultProfile");
        qInfo() << "Default profile cleared.";
        return 0;
    }

    if (parser.isSet(setDefaultProfileOption)) {
        const QString path = parser.value(setDefaultProfileOption);
        if (path.isEmpty() || !QFileInfo::exists(path)) {
            qCritical() << "Specified profile does not exist:" << path;
            return 2;
        }
        const QString absPath = QFileInfo(path).absoluteFilePath();
        settings.setValue("profile/defaultProfile", absPath);
        qInfo().noquote() << "Default profile set to:" << absPath;
        return 0;
    }

    const QStringList positional = parser.positionalArguments();
    if (positional.size() > 1 || (parser.isSet(profileOption) && !positional.isEmpty())) {
        qCritical() << "Specify only one profile, either as --profile FILE or FILE.";
        return 2;
    }
    QString profile = parser.isSet(profileOption) ? parser.value(profileOption)
                                                  : positional.value(0);
    if ((parser.isSet(profileOption) || !positional.isEmpty()) && profile.isEmpty()) {
        qCritical() << "Profile filename must not be empty.";
        return 2;
    }

    if (parser.isSet(headlessOption)) {
        headless = true;
    }

    // Resolve startup profile if none explicitly provided on CLI
    if (profile.isEmpty()) {
        const QString defaultProfile = settings.value("profile/defaultProfile").toString();
        if (!defaultProfile.isEmpty() && QFileInfo::exists(defaultProfile)) {
            profile = defaultProfile;
        } else if (settings.value("profile/reopenLastOnStartup", false).toBool()) {
            const QString lastProfile = settings.value("profile/lastOpenedProfile").toString();
            if (!lastProfile.isEmpty() && QFileInfo::exists(lastProfile)) {
                profile = lastProfile;
            }
        }
    }

    //CREATE MAIN WINDOW
    MainWindow *w = new MainWindow;
    w->resize(1000, 500);

    if (headless) {
        a.setQuitOnLastWindowClosed(false);
        qInfo() << "Running CasterSoundboard in headless mode (display-less).";
    } else {
        w->show();
    }

    if (!profile.isEmpty()) {
        const QString absolutePath = QFileInfo(profile).absoluteFilePath();
        QString error;
        if (!w->loadProfile(absolutePath, error)) {
            qCritical().noquote() << error;
            delete w;
            return 1;
        }
        qInfo().noquote() << "Loaded profile:" << absolutePath;
    }

    int oscPort = -1;
    if (parser.isSet(oscPortOption)) {
        bool ok = false;
        int p = parser.value(oscPortOption).toInt(&ok);
        if (ok && p > 0 && p <= 65535) {
            oscPort = p;
        } else {
            qWarning() << "Invalid --osc-port specified, using default.";
        }
    }

    if (headless || parser.isSet(autoStartOscOption)) {
        if (!w->isOSCServerRunning()) {
            if (w->startOSCServer(oscPort)) {
                qInfo() << "OSC server listening on port" << (oscPort > 0 ? oscPort : 5051);
            } else {
                qWarning() << "Could not start OSC server on port" << (oscPort > 0 ? oscPort : 5051);
            }
        }
    }

    //END APPLICATION
    return a.exec();
}
