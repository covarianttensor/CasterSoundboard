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
#include "CSS.h"
#include <cmath>
#include <QString>

using std::abs;

QString Generate_CSS(HSL_C hsl_color)
{
    RGB_C rgb_color = RGB_C(0,0,0);
    QString CSS_STR = "CasterPlayerWidget{background:qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,stop:0.0 rgba(";
    rgb_color = RGB_C(184,225,252); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(Generate_CSS_Gradient_Stop(0.1));
    rgb_color = RGB_C(169,210,243); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(Generate_CSS_Gradient_Stop(0.25));
    rgb_color = RGB_C(144,186,228); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(Generate_CSS_Gradient_Stop(0.37));
    rgb_color = RGB_C(144,188,234); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(Generate_CSS_Gradient_Stop(0.5));
    rgb_color = RGB_C(144,191,240); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(Generate_CSS_Gradient_Stop(0.51));
    rgb_color = RGB_C(107,168,229); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(Generate_CSS_Gradient_Stop(0.83));
    rgb_color = RGB_C(162,218,245); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(Generate_CSS_Gradient_Stop(1.0));
    rgb_color = RGB_C(189,243,253); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(", 255));}");
    return CSS_STR;
}

QString Generate_CSS_W(HSL_C hsl_color)
{
    RGB_C rgb_color = RGB_C(0,0,0);
    QString CSS_STR = "QWidget{background:qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,stop:0.0 rgba(";
    rgb_color = RGB_C(184,225,252); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(Generate_CSS_Gradient_Stop(0.1));
    rgb_color = RGB_C(169,210,243); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(Generate_CSS_Gradient_Stop(0.25));
    rgb_color = RGB_C(144,186,228); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(Generate_CSS_Gradient_Stop(0.37));
    rgb_color = RGB_C(144,188,234); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(Generate_CSS_Gradient_Stop(0.5));
    rgb_color = RGB_C(144,191,240); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(Generate_CSS_Gradient_Stop(0.51));
    rgb_color = RGB_C(107,168,229); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(Generate_CSS_Gradient_Stop(0.83));
    rgb_color = RGB_C(162,218,245); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(Generate_CSS_Gradient_Stop(1.0));
    rgb_color = RGB_C(189,243,253); rgb_color = rgb_color.addHSL(hsl_color); CSS_STR.append(rgb_color.toStr());
    CSS_STR.append(", 255));}");
    return CSS_STR;
}

QString Generate_CSS_Gradient_Stop(float stop)
{
    QString CSS_STR = ", 255),stop:";
    CSS_STR.append(QString::number(stop));
    CSS_STR.append(" rgba(");
    return CSS_STR;
}

float Clamp(float value, float lower, float upper)
{
    return Max(Min(value, upper), lower);
}

float Max(float a, float b)
{
    if(a == b)
    {
        return a;
    }
    else if(a > b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

float Max(float a, float b, float c)
{
    return Max(Max(a, b), Max(b, c));
}

float Min(float a, float b)
{
    if(a == b)
    {
        return a;
    }
    else if(a > b)
    {
        return b;
    }
    else
    {
        return a;
    }
}

float Min(float a, float b, float c)
{
    return Min(Min(a, b), Min(b, c));
}

float CSS::abs(float a)
{
    if (a >= 0)
    {
        return a;
    }
    else
    {
        return -a;
    }
}

int fSwitch(float value, float a, float b, float c)
{
    if(value == a)
    {
        return 1;
    }
    else if(value == b)
    {
        return 2;
    }
    else if(value == c)
    {
        return 3;
    }
    else
    {
        return 0;
    }
}

RGB_C::RGB_C(HSL_C hsl_color)
{
    float h = hsl_color.H;
    float s = hsl_color.S;
    float l = hsl_color.L;
    float r;
    float g;
    float b;

    float c = ( 1 - CSS::abs( 2 * l - 1 ) ) * s;
    float x = c * ( 1 - CSS::abs( fmod(( h / 60 ), 2 ) - 1 ) );
    float m = l - ( c / 2 );

    if ( h < 60 )
    {
       r = c;
       g = x;
       b = 0;
    }
    else if ( h < 120 )
    {
       r = x;
       g = c;
       b = 0;
    }
    else if ( h < 180 )
    {
       r = 0;
       g = c;
       b = x;
    }
    else if ( h < 240 )
    {
       r = 0;
       g = x;
       b = c;
    }
    else if ( h < 300 )
    {
       r = x;
       g = 0;
       b = c;
    }
    else
    {
       r = c;
       g = 0;
       b = x;
    }

    R = ( r + m ) * 255;
    G = ( g + m ) * 255;
    B = ( b + m  ) * 255;
}

RGB_C RGB_C::operator +(RGB_C &color)
{
    return RGB_C( Clamp(R + color.R, 0, 360),
                  Clamp(G + color.G, 0, 1),
                  Clamp(B + color.B, 0, 1));
}

RGB_C RGB_C::addHSL(HSL_C &color)
{
    RGB_C temp_rgb = RGB_C(R,G,B);
    HSL_C temp_hsl = HSL_C(temp_rgb);
    temp_hsl = temp_hsl + color;
    return RGB_C(temp_hsl);
}

QString RGB_C::toStr()
{
    QString str = "";
    str.append(QString::number(R));
    str.append(",");
    str.append(QString::number(G));
    str.append(",");
    str.append(QString::number(B));
    return str;
}

HSL_C::HSL_C(RGB_C rgb_color)
{
    float r = rgb_color.R / 255.0;
    float g = rgb_color.G / 255.0;
    float b = rgb_color.B / 255.0;
    float max = Max(r, g, b);
    float min = Min(r, g, b);
    float h;
    float s;
    float l = (max + min) / 2.0;
    float d = max - min;
    if(d == 0)
    {
        h = 0;
        s = 0;
    }
    else
    {
        s = d / ( 1 - abs( 2 * l - 1 ) );

        switch(fSwitch(max, r,g,b))
        {
            case 1:
                h = 60 * ( fmod((( g - b ) / d ), 6.0 ) );
                break;
            case 2:
                h = 60 * ( ( b - r ) / d + 2.0 );
                break;
            case 3:
                h = 60 * ( ( r - g ) / d + 4.0 );
                break;
            default:
                h = 0.0;
                break;
        }
    }

    H = h;
    S = s;
    L = l;
}

HSL_C HSL_C::operator +(HSL_C &color)
{
    return HSL_C( Clamp(H + color.H, 0,360),
                  Clamp(S + color.S, 0, 1),
                  Clamp(L + color.L, 0, 1));
}

HSL_C HSL_C::addRGB(RGB_C &color)
{
    HSL_C temp_hsl = HSL_C(H,S,L);
    RGB_C temp_rgb = RGB_C(temp_hsl);
    temp_rgb = temp_rgb + color;
    return HSL_C(temp_rgb);
}

QString HSL_C::toStr()
{
    QString str = "";
    str.append(QString::number(H));
    str.append(",");
    str.append(QString::number(S));
    str.append(",");
    str.append(QString::number(L));
    return str;
}
