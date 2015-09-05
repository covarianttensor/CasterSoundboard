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
#include "CasterLabelColorPicker.h"
#include "CSS.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

//CONSTRUCTOR
CasterLabelColorPicker::CasterLabelColorPicker(QWidget *parent)
{
    //SET WINDOW PROPETIES
    this->setWindowTitle("Select Label Color");
    this->setWindowIcon(QIcon(":/res/img/app.png"));

    //SET CLASS PROPERTIES
    ok = false;
    currentCSS = Generate_CSS(HSL_C(0,0,0));


    //INIT WIDGETS
    label_H = new QLabel; label_H->setText("H:");
    label_S = new QLabel; label_S->setText("S:");
    label_L = new QLabel; label_L->setText("L:");
    label_H_Value = new QLabel; label_H_Value->setText("0");
    label_S_Value = new QLabel; label_S_Value->setText("0");
    label_L_Value = new QLabel; label_L_Value->setText("0");
    slider_H = new QSlider; slider_H->setOrientation(Qt::Horizontal); slider_H->setFixedWidth(200); slider_H->setMinimum(-180); slider_H->setMaximum(180);
    slider_S = new QSlider; slider_S->setOrientation(Qt::Horizontal); slider_S->setFixedWidth(200); slider_S->setMinimum(-100); slider_S->setMaximum(100);
    slider_L = new QSlider; slider_L->setOrientation(Qt::Horizontal); slider_L->setFixedWidth(200); slider_L->setMinimum(-100); slider_L->setMaximum(100);
    widget_Color_Label = new QWidget; widget_Color_Label->setFixedSize(150, 150); widget_Color_Label->setStyleSheet(Generate_CSS_W(HSL_C(0,0,0)));
    button_Cancel = new QPushButton("Cancel"); button_Cancel->setFixedSize(80, 30);
    button_Accept = new QPushButton("Accept"); button_Accept->setFixedSize(80, 30);

    //DENFINE LAYOUT
    QGridLayout *layout = new QGridLayout;
    QHBoxLayout *layout_top = new QHBoxLayout;
    QHBoxLayout *layout_preview = new QHBoxLayout;
    QVBoxLayout *layout_hsl = new QVBoxLayout;
    QHBoxLayout *layout_hsl_h = new QHBoxLayout;
    QHBoxLayout *layout_hsl_s = new QHBoxLayout;
    QHBoxLayout *layout_hsl_l = new QHBoxLayout;
    QHBoxLayout *layout_bottom = new QHBoxLayout;


    //SET LAYOUT
    layout_hsl_h->addWidget(label_H);
    layout_hsl_h->addWidget(slider_H);
    layout_hsl_h->addWidget(label_H_Value);

    layout_hsl_s->addWidget(label_S);
    layout_hsl_s->addWidget(slider_S);
    layout_hsl_s->addWidget(label_S_Value);

    layout_hsl_l->addWidget(label_L);
    layout_hsl_l->addWidget(slider_L);
    layout_hsl_l->addWidget(label_L_Value);

    layout_hsl->addLayout(layout_hsl_h);
    layout_hsl->addLayout(layout_hsl_s);
    layout_hsl->addLayout(layout_hsl_l);

    layout_preview->addWidget(widget_Color_Label);

    layout_top->addLayout(layout_preview);
    layout_top->addLayout(layout_hsl);

    layout_bottom->addWidget(button_Accept, Qt::AlignRight);
    layout_bottom->addWidget(button_Cancel, Qt::AlignRight);

    layout->addLayout(layout_top, 0,0);
    layout->addLayout(layout_bottom, 1,0);

    this->setLayout(layout);

    //SET WINDOW
    this->setModal(true);
    this->setFixedSize(450, 230);
    this->setLayout(layout);

    //MAKE CONNECTIONS
    connect(slider_H,SIGNAL(valueChanged(int)),this,SLOT(slider_HSL_valueChanged(int)));
    connect(slider_S,SIGNAL(valueChanged(int)),this,SLOT(slider_HSL_valueChanged(int)));
    connect(slider_L,SIGNAL(valueChanged(int)),this,SLOT(slider_HSL_valueChanged(int)));
    connect(button_Accept,SIGNAL(clicked()),this,SLOT(button_Accept_Clicked()));
    connect(button_Cancel,SIGNAL(clicked()),this,SLOT(button_Cancel_Clicked()));

}

//SLOTS
void CasterLabelColorPicker::slider_HSL_valueChanged(int value)
{
    label_H_Value->setText(QString::number(slider_H->value()));
    label_S_Value->setText(QString::number(slider_S->value()).append("%"));
    label_L_Value->setText(QString::number(slider_L->value()).append("%"));
    HSL_C hsl_input(slider_H->value(),slider_S->value()/100.0,slider_L->value()/100.0);
    currentCSS = QString(Generate_CSS(hsl_input));
    widget_Color_Label->setStyleSheet(Generate_CSS_W(hsl_input));
}

void CasterLabelColorPicker::button_Accept_Clicked()
{
    ok = true;
    this->close();
}

void CasterLabelColorPicker::button_Cancel_Clicked()
{
    this->close();
}
