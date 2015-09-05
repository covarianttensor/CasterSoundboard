#include "ColorConversion.h"
#include <cstdlib>
#include <cmath>

int *getRGB_fromHLS(int H, int S, int L){
    int *ret = new int[3];

    H = H % 360;
    double S_ = S / 100.0;
    double L_ = L / 100.0;

    double C = (1.0 - abs(2.0*L_-1.0)) * S_;
    double X = C * (1.0 - abs(fmodf(H/60.0, 2.0)-1.0));
    double m = L_ - C/2.0;

    double R_, G_, B_;

    if(0 <= H && H < 60)
    {
        R_ = C;
        G_ = X;
        B_ = 0.0;
    }
    else if (60 <= H && H < 120)
    {
        R_ = X;
        G_ = C;
        B_ = 0.0;
    }
    else if (120 <= H && H < 180)
    {
        R_ = 0.0;
        G_ = C;
        B_ = X;
    }
    else if (180 <= H && H < 240)
    {
        R_ = 0.0;
        G_ = X;
        B_ = C;
    }
    else if (240 <= H && H < 300)
    {
        R_ = X;
        G_ = 0.0;
        B_ = C;
    }
    else if (300 <= H && H < 360)
    {
        R_ = C;
        G_ = 0.0;
        B_ = X;
    }

    ret[0] = (int)(255.0*(R_+m));
    ret[1] = (int)(255.0*(G_+m));
    ret[2] = (int)(255.0*(B_+m));

    return ret;
}
