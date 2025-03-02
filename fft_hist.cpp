#include "fft_hist.h"
#include "string.h"
//#include "memory.h"
#include <time.h>
#include <stdio.h>
//#include <iostream> // library that contain basic input/output functions
//#include <fstream>  // library that contains file input/output functions

using namespace std;

FFT_Hist::FFT_Hist()
{   
    m_alldat = NULL;
    m_avg = NULL;
    m_noise_floor = NULL;
    m_maxvalues = NULL;
    m_minvalues = NULL;
    m_timestamp = NULL;
    //m_numrows = 0;
    m_binsize = 0;
    m_avg_min = 0;
    m_avg_max = 0;
    m_CFHz = 0;
    m_SPS = 0;
    m_emva_alpha = .13;//

    Reset(DEFAULT_BIN_SIZE);
    m_binsize = DEFAULT_BIN_SIZE;
    m_noise_floor_binwidth = DEFAULT_NOISE_FLOOR_BINS;
    m_noise_floor_offset = 0;//DEF_NOISE_FLOOR_OFFSET;
    m_curidx = -1;
    m_rowcnt = 0;
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
        for(int c = 0; c < binsize ; c++)
            m_avg[c] = NOISE_FLOOR;
        m_curidx = -1; // reset back to the first row
        return; // already the right size
    }
    Release(); // free all previously allocated memory
   // printf("FFT_Helper::Reset Binsize = %d\r\n",binsize);
    m_binsize = binsize; //the X size of the bins
    //allocate memory
    m_maxvalues = new float[(unsigned)m_binsize]; // max values is a single row
    m_minvalues = new float[(unsigned)m_binsize]; // min values is a single row
    m_alldat = new float[(unsigned)m_binsize * MAX_FFT_ROWS];
    m_avg = new float[(unsigned)m_binsize];
    m_noise_floor = new float[(unsigned)m_binsize];
    m_timestamp = new long long[MAX_FFT_ROWS];
    ClearMinMaxValues();
    for(int c = 0; c < binsize ; c++)
        m_avg[c] = NOISE_FLOOR;
    m_curidx = -1; // reset back to the first row
    m_rowcnt = 0;
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
    if(m_timestamp != NULL)
    {
        delete []m_timestamp;
        m_timestamp = NULL;
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

void FFT_Hist::AddData(float *fft, int numbins, float centerfreq,float SPS, long long time_uSecond)
{
    Lock();

    if(numbins != m_binsize) // support a changing bin size if needed
    {
        Reset(numbins);
    }

    if(centerfreq != m_CFHz || m_SPS != SPS)
    {
        m_CFHz = centerfreq; // set the new CF
        m_SPS  = SPS;
        ClearMinMaxValues();
        m_avg_min =fft[0]; // get the first entry as a starting point
        m_avg_max =fft[0];
    }

    //increment the m_curidx
    m_curidx++;
    //    if(m_curidx == MAX_FFT_ROWS)
    //        m_curidx = -1; // roll over

        if(m_curidx == MAX_FFT_ROWS)
            m_curidx = 0; // roll over

   // cout << m_curidx << "\r\n";
  //  printf("curidx %d\r\n",m_curidx);
    int rowsz = m_binsize * sizeof(float);    // size of 1 row in bytes
    //copy the fft data into the specified row
    memcpy(&m_alldat[m_binsize * m_curidx],fft,rowsz);
    if(m_rowcnt == 0) //if this is the fist row, copy it over to the average
    {
        memcpy(m_avg,fft,rowsz);
    }
    CalcAvgEMA(fft); // calculate the exponential moving average
    CalcMinMax(fft); // look for the high-water marks on the instantaneous FFT values
    m_rowcnt++;
    if(m_rowcnt > MAX_FFT_ROWS)
        m_rowcnt = MAX_FFT_ROWS;
    Unlock();


}
void FFT_Hist::CopyTo(float * dest) // copy the entire m_alldat to the specified dest in reverse order for the waterfall
{
    try
    {
    if(m_curidx == -1)
        return;

    //let's do this row by row..
    int sourcerow = m_curidx;
    if(sourcerow == 256)
        return;
    for(int c = 0 ; c < MAX_FFT_ROWS; c++)
    {
        float * src;
        src = &m_alldat[sourcerow * m_binsize];
        memcpy(&dest[c * m_binsize],src,m_binsize * sizeof(float));
        sourcerow --;
        if(sourcerow < 0)
            sourcerow = MAX_FFT_ROWS;
    }
    }catch(...){}
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
    val -= (m_SPS / 2.0f);
    return val;
}

float FFT_Hist::GetHighFreqHz()
{
    float val = m_CFHz;
    val += (m_SPS / 2.0f);
    return val;
}

// return freq in hz from 0 -> DEFAULT_BIN_SIZE-1 value
float FFT_Hist::GetFreqHz(int binidx) // in Hz
{
    float leftfreq = GetLowFreqHz();
    float freqinc = ((float)m_SPS) / ((float)GetBinSize());
    return leftfreq + (binidx * freqinc);
}

// return the closest bin index to the frequency or -1
int FFT_Hist::GetBinIndex(float freqHz)
{
    //long leftfreq = m_centerfreq - (m_spanHz / 2);
    //long rightfreq = m_centerfreq + (m_spanHz / 2);
    float freqinc = m_SPS / (float)GetBinSize();
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

float *FFT_Hist::GetAvgRow() // get specified row of data (waterfall) average data
{
    return m_avg;
}

/*
calculate the noise floor from the average of the bins
*/
void FFT_Hist::CalcNoiseFloor(float *stddev)
{
    int binwidth = m_noise_floor_binwidth;

    float   sum_of_sq = 0;
    for(int c = 0; c < GetBinSize(); c++ )
    {
        sum_of_sq = 0;
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
            sum_of_sq += fabs(m_avg[i]) * fabs(m_avg[i]);
        }
        mean /= range;
        if(stddev != nullptr)
        {
            stddev[c] = sqrt( sum_of_sq / range - mean*mean );
        }
        m_noise_floor[c] = mean + m_noise_floor_offset;
    }
}

QMap<int,float> FFT_Hist::DetectPeaks(float det_thresh_scaler)
{
    //det_thresh_scaler = 3.25f; // a multiple of the standard deviation is the threshold point
    QMap<int,float> peaks; // the detected peaks
    int numbins = GetBinSize();
    float tdat[numbins];
    float *data = AvgData();
    float   mean = 0;
    float   stdev = 0;
    //storage for standard deviation
    float stddev[numbins];

    CalcNoiseFloor(stddev); // calc noise floor and standard deviation

    int lastPeak = -1;
  //  printf("DetectPeaks mean:%f , sumofsq:%f , stddev:%f\r\n",mean,sum_of_sq, stdev );
    //get the abs of the average values
    for(int c = 0; c < GetBinSize(); c++)
    {
        tdat[c] = fabs(data[c]);
    }

    for (int bin = 0; bin < numbins; bin++)
    {
        mean = fabs(m_noise_floor[bin]);
        //det_thresh_scaler times the std over the mean or better than current peak
        float d = (lastPeak == -1) ? (mean - det_thresh_scaler * stdev) :  tdat[lastPeak];

        if (tdat[bin] < d)
            lastPeak = bin;

        if (lastPeak != -1 && (bin - lastPeak > PEAK_H_TOLERANCE || bin == numbins - 1))
        {
            peaks.insert(lastPeak, tdat[lastPeak]);
            lastPeak = -1;
        }
    }
    return peaks;
}
