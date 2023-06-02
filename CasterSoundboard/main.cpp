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
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "casterplayer_controller.h"
#include "casterplayer_model.h"

int main(int argc, char *argv[])
{
    //Create application
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    //Register Types
    qmlRegisterType<CasterPlayerModel>("MVC_CasterPlayer", 1, 0, "CasterPlayerModel");
    qmlRegisterUncreatableType<CasterPlayerController>("MVC_CasterPlayer", 1, 0, "CasterPlayerController",
            QStringLiteral("CasterPlayerController should not be created in QML"));

    //Create QML engine instance
    QQmlApplicationEngine engine;

    //Load context properties
    CasterPlayerController casterPlayerController;
    engine.rootContext()->setContextProperty(QStringLiteral("casterPlayerController"), &casterPlayerController);

    //Start QML engine with main qml file
    engine.load(QUrl(QLatin1String("qrc:/qml/views/mainView.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();

}
