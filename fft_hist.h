#ifndef FFT_HELPER_H
#define FFT_HELPER_H

#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <cstdio>
#include <fstream>

#define MAX_FFT_ROWS 256 // keep the last 256 rows of data
#define PEAK_H_TOLERANCE 2

#define DEFAULT_BIN_SIZE 1024
#define MAX_FFT_SIZE 16384

#define DEFAULT_AVG_LEN 10 // use last X rows of data to calculate average
#define DEFAULT_FFT_UPDATE_RATE 500.0 // the default update rate
#define DEFAULT_NOISE_FLOOR_BINS 10


extern float FFT_UPDATE_RATE;// =  DEFAULT_FFT_UPDATE_RATE; // in hz
extern float uS_PER_ROW;//
extern float TIME_RESOLUTION; // 1000 for ms, 1000000 for us

extern int FFT_BIN_SIZE;

#define Pow2Mw(pwrIn) (pow(10,(pwrIn)/10))
#define Mw2Pow(mwIn) (10*log10(mwIn))

class FFT_Hist
{
public:

    FFT_Hist();
    ~FFT_Hist();
    void Release(); // release allocated memory
    int GetBinSize();

    void AddData(float *fftData, int numbins, float centerfreq,float BWHz);//, unsigned long sampleidx, long ts=0);

    float *AvgData(){return m_avg;} // return the power averaged data
    float *RawData(); // return the last entered raw data of samples
    float *MaxValues(); // return the high-water marks
    float *MinValues(); // return the low-water marks

    // Set and get the number of rows to average
    void SetAverageLength(int avg_rows){m_avg_rows = avg_rows;}
    int GetAverageLength(){return m_avg_rows;}

    int MaxRows(); // returns the maxinum number of rows in this plot
    float *GetRow(int row); // get specified row of data (waterfall)
    float *GetAvgRow(); // get specified row of data (waterfall) average data
    float *GetNoiseFloor(){return m_noise_floor;}

    void Set(float cfhz, float bwhz);
    void Reset(int binsize);
    void ClearMinMaxValues();
    float GetFreqHz(int binidx); // in hz
    float GetTime(int row); // get the time index at the given row (uS) from the uS_PER_ROW
    float GetLowFreqHz();
    float GetHighFreqHz();
    float GetCenterFreq(){return m_CFHz;}
    float GetMinDBM(bool unbounded = false); // for graphing, changes when new data is added, reset when freq or span changes
    float GetMaxDBM(); // for graphing, changes when new data is added, reset when freq or span changes
    float GetMinDBMRounded(); // for graphing, changes when new data is added, reset when freq or span changes
    float GetMaxDBMRounded(); // for graphing, changes when new data is added, reset when freq or span changes

    float GetBWHz(){return GetHighFreqHz() - GetLowFreqHz();}
    float GetRBWHz(){return GetBWHz() / (float)m_binsize;}
    void SetMaxRows(int maxrows);
    int GetBinIndex(float freqHz); // return the closest bin index to the frequency or -1
    void CalcMinMax(float *vals); // line vals
    void CalcNoiseFloor();


    void Lock(){m_mutex.lock();}
    void Unlock(){m_mutex.unlock();}
    void CalcAvgEMA(float *linedat);

    float m_avg_min; // valid until the freq / span changes
    float m_avg_max; // valid until the freq / span changes

    int m_avg_rows; // how many rows to average
    int m_noise_floor_binwidth;
    double m_noise_floor_offset;

    float m_emva_alpha; // exponential moving average alpha value (0 - 1)
    void SetAlpha(float val){m_emva_alpha = val;}
    float GetAlpha(){return m_emva_alpha;}

    int noise_floor_binwidth() const;
    void setNoise_floor_binwidth(int noise_floor_binwidth);

    double noise_floor_offset() const;
    void setNoise_floor_offset(double noise_floor_offset);

private:

    int m_binsize; // number of bins (X table size)
    float *m_alldat; // a DEFAULT_BIN_SIZE * MAX_FFT_ROWS table image of values
    float *m_avg; // a table of averaged data
    float *m_noise_floor; // a dynamic noise floor
    //max value tracking
    float *m_maxvalues; // a single row of max values (need to find some way to make them fade / remove)
    float *m_minvalues; // a single row of max values (need to find some way to make them fade / remove)
    //long   *m_timestamp; // the time this value was updated last

    float m_CFHz; // in Hz - the center frequency
    float m_BWHz; // in Hz - also the SPS
    boost::mutex m_mutex; // for locking the tables to avoind threading issues

};

#endif // FFT_HELPER_H
