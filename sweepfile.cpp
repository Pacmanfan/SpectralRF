#include "sweepfile.h"


SweepDataLine::SweepDataLine(int numbins)
{
    m_timestamp=0;// the timestamp of when this sweep was completed
    m_data = new float[numbins];
    m_numbins = numbins; // total number of bins across the sweepline
}

void SweepDataLine::Save(FILE *fp)
{

}

void SweepDataLine::Load(FILE *fp)
{

}


SweepFile::SweepFile()
{

}

bool SweepFile::OpenSave(string filename)
{
    return false;
}

bool SweepFile::OpenLoad(string filename)
{
    return false;
}

void SweepFile::Close()
{

}

bool SweepFile::LoadNext(SweepDataLine *line)
{
    //return false for eof
    return false;
}
