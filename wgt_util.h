#ifndef WGT_UTIL_H
#define WGT_UTIL_H
#include <cstdlib>
//utility classes and functions for widget controls


#define MXTHRESH 10
#define MYTHRESH 10
#define LINEGRIPWIDTH 10 // pixels
#define VLINEWIDTH 4 //pixels
#define ZOOMFACTOR 1.15

class mouseinfo
{
public:
    int lx,ly;
    int mx,my;
    bool rb,lb,mb;
    mouseinfo();
    void Set(int x, int y)
    {
        lx = mx; // save the last x
        ly = my; // save the last y
        mx = x;my=y; // upodate the current
    }
    bool DiffPastThreshold();
};

/*
Range animation helper for zooming in and out smoothly
*/
class RangeAnimHelper
{
public :
    double _incscale;
    int _count;
    RangeAnimHelper()
    {
        _count = 0;
        _incscale = 1.0;
    }
    void Setup(double incscale,int count)
    {
        _incscale = incscale;
        _count = count;
    }
};

#endif // WGT_UTIL_H
