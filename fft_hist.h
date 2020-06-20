#ifndef FFT_HELPER_H
#define FFT_HELPER_H

#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <cstdio>
#include <fstream>
#include <QMap>

#define MAX_FFT_ROWS 256 // keep the last 256 rows of data
#define PEAK_H_TOLERANCE 2

#define DEFAULT_BIN_SIZE 1024
#define NOISE_FLOOR -130.0f
#define DEF_NOISE_FLOOR_OFFSET 3 // 3db offset
#define DEFAULT_NOISE_FLOOR_BINS 10
#define Pow2Mw(pwrIn) (pow(10,(pwrIn)/10))
#define Mw2Pow(mwIn) (10*log10(mwIn))

class FFT_Hist
{
public:

    FFT_Hist();
    ~FFT_Hist();
    void Release(); // release allocated memory
    int GetBinSize();
    void AddData(float *fftData, int numbins, float centerfreq,float SPS, long long time_uSecond);

    float *AvgData(){return m_avg;} // return the power averaged data
    float *MaxValues(); // return the high-water marks
    float *MinValues(); // return the low-water marks

    int MaxRows(); // returns the maxinum number of rows in this plot
   // float *GetRow(int row); // get specified row of data (waterfall)
    void CopyTo(float * dest); // copy the entire m_alldat to the specified dest
    float *GetAvgRow(); // get specified row of data (waterfall) average data
    float *GetNoiseFloor(){return m_noise_floor;}

    void Reset(int binsize);
    void ClearMinMaxValues();
    float GetFreqHz(int binidx); // in hz
    float GetLowFreqHz();
    float GetHighFreqHz();

    float GetMinDBM(bool unbounded = false); // for graphing, changes when new data is added, reset when freq or span changes
    float GetMaxDBM(); // for graphing, changes when new data is added, reset when freq or span changes

    float GetBWHz(){return GetHighFreqHz() - GetLowFreqHz();}
    float GetRBWHz(){return GetBWHz() / (float)m_binsize;}

    int GetBinIndex(float freqHz); // return the closest bin index to the frequency or -1
    void CalcMinMax(float *vals); // line vals
    void CalcNoiseFloor(float *stddev = 0);

    void Lock(){m_mutex.lock();}
    void Unlock(){m_mutex.unlock();}
    void CalcAvgEMA(float *linedat);

    float m_avg_min; // valid until the freq / span changes
    float m_avg_max; // valid until the freq / span changes

    void SetAlpha(float val){m_emva_alpha = val;}
    float GetAlpha(){return m_emva_alpha;}

    int noise_floor_binwidth() const;
    void setNoise_floor_binwidth(int noise_floor_binwidth);

    double noise_floor_offset() const;
    void setNoise_floor_offset(double noise_floor_offset);
    QMap<int,float> DetectPeaks(float PeakDetection = 3.25);

private:
    int m_curidx; // the current row index
    int m_noise_floor_binwidth; // the number of bins to average together to get a noise floor
    double m_noise_floor_offset; // the db offset of the noise floor
    float m_emva_alpha; // exponential moving average alpha value (0 - 1)
    int m_binsize; // number of bins (X table size)
    float *m_alldat; // a DEFAULT_BIN_SIZE * MAX_FFT_ROWS table image of values
    float *m_avg; // a single row of averaged data
    float *m_noise_floor; // a single row of dynamic noise floor
    float *m_maxvalues; // a single row of max values (need to find some way to make them fade / remove)
    float *m_minvalues; // a single row of max values (need to find some way to make them fade / remove)
    long long  *m_timestamp; // MAX_FFT_ROWS array of the time of the rows in uS
    int m_rowcnt;

    float m_CFHz; // in Hz - the center frequency
    float m_SPS; // in Hz - also the SPS
    boost::mutex m_mutex; // for locking the tables to avoind threading issues

};

#endif // FFT_HELPER_H
