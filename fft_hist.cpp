#include "fft_hist.h"
#include "string.h"
#include "memory.h"
#include <time.h>
#include <stdio.h>
#include <iostream> // library that contain basic input/output functions
#include <fstream>  // library that contains file input/output functions

using namespace std;

#define NOISE_FLOOR -130.0f
float FFT_UPDATE_RATE =  DEFAULT_FFT_UPDATE_RATE; // in hz
float TIME_RESOLUTION = 1000000.0;
float uS_PER_ROW = (TIME_RESOLUTION/FFT_UPDATE_RATE);
#define DEF_NOISE_FLOOR_OFFSET 3 // 3db offset


FFT_Hist::FFT_Hist()
{   
    m_alldat = NULL;
    m_avg = NULL;
    m_noise_floor = NULL;
    m_maxvalues = NULL;
    m_minvalues = NULL;
    //m_numrows = 0;
    m_binsize = 0;
    m_avg_min = 0;
    m_avg_max = 0;
    m_CFHz = 0;
    m_BWHz = 0;
    m_emva_alpha = .13;//

    m_avg_rows = DEFAULT_AVG_LEN;
    Reset(MAX_FFT_SIZE);
    m_binsize = DEFAULT_BIN_SIZE;
    m_noise_floor_binwidth = DEFAULT_NOISE_FLOOR_BINS;
    m_noise_floor_offset = DEF_NOISE_FLOOR_OFFSET;
}

FFT_Hist::~FFT_Hist()
{
    Release();
}

void FFT_Hist::Reset(int binsize)
{
    if(m_binsize == binsize)
    {
        ClearMinMaxValues();
        for(int c = 0; c < binsize ; c++) m_avg[c] = NOISE_FLOOR;
        return; // already the right size
    }
    Release(); // free all previously allocated memory
   // printf("FFT_Helper::Reset Binsize = %d\r\n",binsize);
    m_binsize = binsize; //the X size of the bins
    //allocate memory
    m_maxvalues = new float[(unsigned)m_binsize]; // max values is a single row
    m_minvalues = new float[(unsigned)m_binsize]; // min values is a single row
    m_alldat = new float[(unsigned)m_binsize * MAX_FFT_ROWS];
    m_avg = new float[(unsigned)m_binsize];//* MAX_FFT_ROWS]; //
    m_noise_floor = new float[(unsigned)m_binsize];//* MAX_FFT_ROWS]; //
    ClearMinMaxValues();
    for(int c = 0; c < binsize ; c++) m_avg[c] = NOISE_FLOOR;
}

void FFT_Hist::Release() // release allocated memory
{
    if(m_alldat != NULL )
    {
        delete []m_alldat;
        m_alldat = NULL;
    }

    if(m_avg != NULL )
    {
        delete []m_avg;
        m_avg = NULL;
    }
    if(m_noise_floor != NULL )
    {
        delete []m_noise_floor;
        m_noise_floor = NULL;
    }

    if(m_maxvalues != NULL )
    {
        delete []m_maxvalues;
        m_maxvalues = NULL;
    }
    if(m_minvalues != NULL )
    {
        delete []m_minvalues;
        m_minvalues = NULL;
    }
}

int FFT_Hist::GetBinSize()
{
    return m_binsize;
}

void FFT_Hist::ClearMinMaxValues()
{
    for(int c=0 ; c < m_binsize ; c++)
    {
        m_maxvalues[c] = NOISE_FLOOR;// set to noise floor
        m_minvalues[c] = 0;
    }
}

float FFT_Hist::GetMinDBM(bool unbounded)
{
    if(unbounded)
        return m_avg_min;
    //find the nearest 10 below this
    if(m_avg_min < NOISE_FLOOR)
    {
        return NOISE_FLOOR; // cap it at -130
    }else{
        return m_avg_min;
    }
}

float FFT_Hist::GetMaxDBM()
{
    return m_avg_max;
}
/*
float FFT_Hist::GetMinDBMRounded() // for graphing, changes when new data is added, reset when freq or span changes
{
    int number = m_avg_min;
    int multiple = 10.0;
    int result = ((number + multiple/2.0) / multiple) * multiple;
    if(result < NOISE_FLOOR)
        return NOISE_FLOOR;
    return result;
}

float FFT_Hist::GetMaxDBMRounded() // for graphing, changes when new data is added, reset when freq or span changes
{
    int number = m_avg_max;
    int multiple = 10.0;
    int result = ((number + ceil(multiple/2.0)) / multiple) * multiple;
    return result;
}
*/
void FFT_Hist::Set(float centerfreq,float BWHz)
{
    m_CFHz = centerfreq; // set the new CF
    m_BWHz  = BWHz;
}

void FFT_Hist::AddData(float *fft, int numbins, float centerfreq,float BWHz)
{
  //  Lock();

    if(numbins != m_binsize) // support a changing bin size if needed
    {
        m_binsize = numbins;
        ClearMinMaxValues();
        memset(m_alldat,0,MAX_FFT_SIZE * MAX_FFT_ROWS);
    }

    if(centerfreq != m_CFHz || m_BWHz != BWHz)
    {
        m_CFHz = centerfreq; // set the new CF
        m_BWHz  = BWHz;

        ClearMinMaxValues();
        m_avg_min =fft[0]; // get the first entry as a starting point
        m_avg_max =fft[0];
    }

    unsigned char *dst,*src; // destination and start pointers in bytes here
    int szmv,rowsz;    
    rowsz = m_binsize * sizeof(float);    //calculate the size of the row in bytes
//    szmv = rowsz * ( MAX_FFT_ROWS - 1);    // the block size is the row size * the number of rows -1
    szmv = rowsz * ( MAX_FFT_ROWS - 2);    // the block size is the row size * the number of rows -1
    src = (unsigned char *)m_alldat; // the source is the begining of the all data block
    dst = src + rowsz;
    memmove(dst,src,szmv);    
    CalcAvgEMA(fft);
    CalcMinMax(fft); // look for the high-water marks
    memcpy(m_alldat,fft,m_binsize * sizeof(float));//copy the new data to the first row
  //  Unlock();
}

/*
The idea is that the high-level will be marked, then fade over time
until it goes back to the current level
*/
void FFT_Hist::CalcMinMax(float *vals)
{

    for(int x = 0; x < m_binsize;x++)
    {
        // examine latest row of data
        if(vals[x] > m_maxvalues[x])
        {
            m_maxvalues[x] = vals[x];
        }
        if(vals[x] < m_minvalues[x])
        {
            m_minvalues[x] = vals[x];
        }
        /*
        if(vals[x] > m_avg_max)
        {
            m_avg_max = vals[x];
        }
        */
    }
}


/*
need a new function to calculate the exponential moving average
https://stackoverflow.com/questions/10990618/calculate-rolling-moving-average-in-c
You pick a constant "alpha" that is between 0 and 1, and compute this:
accumulator = (alpha * new_value) + (1.0 - alpha) * accumulator
You just need to find a value of "alpha" where the effect of a given sample only lasts for about 1000 samples.
*/
void FFT_Hist::CalcAvgEMA(float *linedat)
{

    for(int x = 0; x < m_binsize;x++)
    {
        //accumulator = (alpha * new_value) + (1.0 - alpha) * accumulator
        m_avg[x] = ((m_emva_alpha * linedat[x]) + (1.0 - m_emva_alpha) * m_avg[x]); //initialize the value to 0

        if(x ==0 )
        {
            m_avg_min = m_avg[x];
            m_avg_max = m_avg[x];
        }
        if(m_avg[x] < m_avg_min) m_avg_min = m_avg[x];
        if(m_avg[x] > m_avg_max) m_avg_max = m_avg[x];
    }
}

double FFT_Hist::noise_floor_offset() const
{
    return m_noise_floor_offset;
}

void FFT_Hist::setNoise_floor_offset(double noise_floor_offset)
{
    m_noise_floor_offset = noise_floor_offset;
}

int FFT_Hist::noise_floor_binwidth() const
{
    return m_noise_floor_binwidth;
}

void FFT_Hist::setNoise_floor_binwidth(int noise_floor_binwidth)
{
    m_noise_floor_binwidth = noise_floor_binwidth;
}

float *FFT_Hist::MaxValues()
{
    return m_maxvalues;
}
float *FFT_Hist::MinValues()
{
    return m_minvalues;
}

float FFT_Hist::GetLowFreqHz()
{
    float val = m_CFHz;
    val -= (m_BWHz / 2.0f);
    return val;
}

float FFT_Hist::GetHighFreqHz()
{
    float val = m_CFHz;
    val += (m_BWHz / 2.0f);
    return val;
}
 // get the time index at the given row (uS) from the uS_PER_ROW
// This is the relative row time, not the absolute row time
float FFT_Hist::GetTime(int row)
{
    float timeuS = (float)row *uS_PER_ROW;
    return timeuS;
}

// return freq in hz from 0 -> DEFAULT_BIN_SIZE-1 value
float FFT_Hist::GetFreqHz(int binidx) // in Hz
{
    float leftfreq = GetLowFreqHz();
    float freqinc = ((float)m_BWHz) / ((float)GetBinSize());
    return leftfreq + (binidx * freqinc);
}

// return the closest bin index to the frequency or -1
int FFT_Hist::GetBinIndex(float freqHz)
{
    //long leftfreq = m_centerfreq - (m_spanHz / 2);
    //long rightfreq = m_centerfreq + (m_spanHz / 2);
    float freqinc = m_BWHz / (float)GetBinSize();
    int binidx = 0;
    if(freqHz >= GetLowFreqHz() && freqHz <= GetHighFreqHz())
    {
        //can I calculate the index directly?
        float diff_freq = freqHz - GetLowFreqHz();
        binidx = (int)(diff_freq / freqinc);
        return binidx;
    }else
    {
        return -1;
    }
}

int FFT_Hist::MaxRows()
{
    return MAX_FFT_ROWS;
}

float *FFT_Hist::GetRow(int row) // get specified row of data (waterfall)
{
    float *ptr = m_alldat;
    ptr += (row * m_binsize); // index into the correct pointer row
    return ptr;
}

float *FFT_Hist::GetAvgRow() // get specified row of data (waterfall) average data
{
    return m_avg;
}

/*
calculate the noise floor from the average of the bins
*/
void FFT_Hist::CalcNoiseFloor()
{
    int binwidth = m_noise_floor_binwidth;
    //int idx = 0;
    for(int c = 0; c < GetBinSize(); c++ )
    {
        int lowidx = c - binwidth;
        int highidx = c + binwidth;
        float mean = 0;

        if(lowidx < 0 )lowidx = 0;
        if(highidx > GetBinSize())
            highidx = GetBinSize();

        int range = highidx - lowidx;

        for (int i = lowidx; i < highidx; i++)
        {
            mean += m_avg[i];
        }
        mean /= range;
        m_noise_floor[c] = mean + m_noise_floor_offset;
    }
}
