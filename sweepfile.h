#ifndef SWEEPFILE_H
#define SWEEPFILE_H

#include <stdio.h>
#include <string>

using namespace std;


struct SweepDataLine
{
    SweepDataLine(int numbins);
    long long m_timestamp;// the timestamp of when this sweep was completed in nanoseconds
    float *m_data;
    int m_numbins; // total number of bins across the sweepline
    void Save(FILE *fp);
    void Load(FILE *fp);
};


/*
This class is a recording of an FFT sweep
*/

class SweepFile
{
public:
    SweepFile();
    bool OpenSave(string filename);
    bool OpenLoad(string filename);
    void Close();
    bool LoadNext(SweepDataLine *line);
    std::string m_filename;
    FILE *m_fp;
};

#endif // SWEEPFILE_H
