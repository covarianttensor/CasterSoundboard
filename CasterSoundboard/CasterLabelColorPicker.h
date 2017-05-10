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
#ifndef CASTERLABELCOLORPICKER_H
#define CASTERLABELCOLORPICKER_H
#include <QDialog>

//forward declarations
class QWidget;
class QSlider;
class QLineEdit;
class QPushButton;
class QLabel;
class QString;

class CasterLabelColorPicker : public QDialog //inherit from QWidget
{
    Q_OBJECT
public:
    //Constructor
    CasterLabelColorPicker();

    //PROPERTIES
    bool ok;
    QString currentCSS;
    int slider_H_Value;
    int slider_S_Value;
    int slider_L_Value;

    //METHODS
    void set_slider_H_Value(int H_Value);
    void set_slider_S_Value(int S_Value);
    void set_slider_L_Value(int L_Value);

    //WIDGETS


protected:
    //PROPERTIES

    //METHODS

    //WIDGETS

private:
    //PROPERTIES

    //METHODS

    //WIDGETS
    QLabel *label_H;
    QLabel *label_S;
    QLabel *label_L;
    QLabel *label_H_Value;
    QLabel *label_S_Value;
    QLabel *label_L_Value;
    QSlider *slider_H;
    QSlider *slider_S;
    QSlider *slider_L;
    QWidget *widget_Color_Label;
    QPushButton *button_Cancel;
    QPushButton *button_Accept;


signals:
    //SIGNALS

public slots:
    //SLOTS
    void slider_HSL_valueChanged(int value);
    void button_Accept_Clicked();
    void button_Cancel_Clicked();

};
#endif // CASTERLABELCOLORPICKER_H
