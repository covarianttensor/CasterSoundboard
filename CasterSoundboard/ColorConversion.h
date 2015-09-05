#ifndef COLORCONVERSION_H
#define COLORCONVERSION_H
#include <stdlib.h>
#include <stdio.h>

struct RGB {
  uint r;
  uint g;
  uint b;
} ;

struct HSL {
  uint h;
  uint s;
  uint l;
} ;

void HSLtoRGB_Subfunction(uint& c, const double& temp1, const double& temp2, const double& temp3);

// This function extracts the hue, saturation, and luminance from "color"
// and places these values in h, s, and l respectively.
HSL RGBtoHSL(const RGB& rgb)
{
    HSL hsl;

    uint r = rgb.r;
    uint g = rgb.g;
    uint b = rgb.b;

    double r_percent = ((double)r)/255;
    double g_percent = ((double)g)/255;
    double b_percent = ((double)b)/255;

    double max_color = 0;
    if((r_percent >= g_percent) && (r_percent >= b_percent))
        max_color = r_percent;
    if((g_percent >= r_percent) && (g_percent >= b_percent))
        max_color = g_percent;
    if((b_percent >= r_percent) && (b_percent >= g_percent))
        max_color = b_percent;

    double min_color = 0;
    if((r_percent <= g_percent) && (r_percent <= b_percent))
        min_color = r_percent;
    if((g_percent <= r_percent) && (g_percent <= b_percent))
        min_color = g_percent;
    if((b_percent <= r_percent) && (b_percent <= g_percent))
        min_color = b_percent;

    double L = 0;
    double S = 0;
    double H = 0;

    L = (max_color + min_color)/2;

    if(max_color == min_color)
    {
        S = 0;
        H = 0;
    }
    else
    {
        if(L < .50)
        {
            S = (max_color - min_color)/(max_color + min_color);
        }
        else
        {
            S = (max_color - min_color)/(2 - max_color - min_color);
        }
        if(max_color == r_percent)
        {
            H = (g_percent - b_percent)/(max_color - min_color);
        }
        if(max_color == g_percent)
        {
            H = 2 + (b_percent - r_percent)/(max_color - min_color);
        }
        if(max_color == b_percent)
        {
            H = 4 + (r_percent - g_percent)/(max_color - min_color);
        }
    }
    hsl.s = (uint)(S*100);
    hsl.l = (uint)(L*100);
    H = H*60;
    if(H < 0)
        H += 360;
    hsl.h = (uint)H;
}

// This function converts the "color" object to the equivalent RGB values of
// the hue, saturation, and luminance passed as h, s, and l respectively
RGB HSLtoRGB(const HSL& hsl)
{
    RGB rgb;

    uint r = 0;
    uint g = 0;
    uint b = 0;

    uint h = hsl.h;
    uint s = hsl.s;
    uint l = hsl.l;

    double L = ((double)hsl.l)/100;
    double S = ((double)hsl.s)/100;
    double H = ((double)hsl.h)/360;

    if(s == 0)
    {
        r = l;
        g = l;
        b = l;
    }
    else
    {
        double temp1 = 0;
        if(L < .50)
        {
            temp1 = L*(1 + S);
        }
        else
        {
            temp1 = L + S - (L*S);
        }

        double temp2 = 2*L - temp1;

        double temp3 = 0;
        for(int i = 0 ; i < 3 ; i++)
        {
            switch(i)
            {
            case 0: // red
                {
                    temp3 = H + .33333;
                    if(temp3 > 1)
                        temp3 -= 1;
                    HSLtoRGB_Subfunction(r,temp1,temp2,temp3);
                    break;
                }
            case 1: // green
                {
                    temp3 = H;
                    HSLtoRGB_Subfunction(g,temp1,temp2,temp3);
                    break;
                }
            case 2: // blue
                {
                    temp3 = H - .33333;
                    if(temp3 < 0)
                        temp3 += 1;
                    HSLtoRGB_Subfunction(b,temp1,temp2,temp3);
                    break;
                }
            default:
                {

                }
            }
        }
    }
    rgb.r = (uint)((((double)r)/100)*255);
    rgb.g = (uint)((((double)g)/100)*255);
    rgb.b = (uint)((((double)b)/100)*255);
}

// This is a subfunction of HSLtoRGB
void HSLtoRGB_Subfunction(uint& c, const double& temp1, const double& temp2, const double& temp3)
{
    if((temp3 * 6) < 1)
        c = (uint)((temp2 + (temp1 - temp2)*6*temp3)*100);
    else
    if((temp3 * 2) < 1)
        c = (uint)(temp1*100);
    else
    if((temp3 * 3) < 2)
        c = (uint)((temp2 + (temp1 - temp2)*(.66666 - temp3)*6)*100);
    else
        c = (uint)(temp2*100);
    return;
}


#endif // COLORCONVERSION_H
