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
#ifndef CSS_H
#define CSS_H

#define CSS_GRAIENT_GREY "CasterPlayerWidget{background:qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,stop:0.0 rgba(218,218,218, 255),stop:0.1 rgba(206,206,206, 255),stop:0.25 rgba(186,186,186, 255),stop:0.37 rgba(189,189,189, 255),stop:0.5 rgba(192,192,192, 255),stop:0.51 rgba(168,168,168, 255),stop:0.83 rgba(204,204,204, 255),stop:1.0 rgba(221,221,221, 255));}"
#define CSS_GRAIENT_GREY_W "QWidget{background:qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,stop:0.0 rgba(218,218,218, 255),stop:0.1 rgba(206,206,206, 255),stop:0.25 rgba(186,186,186, 255),stop:0.37 rgba(189,189,189, 255),stop:0.5 rgba(192,192,192, 255),stop:0.51 rgba(168,168,168, 255),stop:0.83 rgba(204,204,204, 255),stop:1.0 rgba(221,221,221, 255));}"

//Foward Declarations
class QString;
struct RGB_C;
struct HSL_C;

struct RGB_C {
    float R;
    float G;
    float B;
    RGB_C(float red, float green, float blue) : R(red), G(green), B(blue) {}
    RGB_C(HSL_C hsl_color);
    RGB_C operator+(RGB_C &color);
    RGB_C addHSL(HSL_C &color);
    QString toStr();
} ;

struct HSL_C {
    float H;
    float S;
    float L;
    HSL_C(float hue, float sat, float light ) : H(hue), S(sat), L(light) {}
    HSL_C(RGB_C rgb_color);
    HSL_C operator+(HSL_C &color);
    HSL_C addRGB(RGB_C &color);
    QString toStr();
};

QString Generate_CSS(HSL_C hsl_color);
QString Generate_CSS_W(HSL_C hsl_color);
QString Generate_CSS_Gradient_Stop(float stop);

float Clamp(float value, float lower, float upper);
float Max(float a, float b);
float Max(float a, float b, float c);
float Min(float a, float b);
float Min(float a, float b, float c);
namespace CSS {
float abs(float a);
}
int fSwitch(float value, float a, float b, float c);

#endif // CSS_H
