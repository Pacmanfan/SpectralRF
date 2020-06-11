#include "wgt_util.h"

//mouse helper class function
bool mouseinfo::DiffPastThreshold()
{
    if(abs((mx - lx)) > MXTHRESH)
    {
        return true;
    }
    if(abs((my - ly)) > MYTHRESH)
    {
        return true;
    }
    return false;
}

mouseinfo::mouseinfo()
{
    rb = false;
    mb = false;
    lb = false;
}
