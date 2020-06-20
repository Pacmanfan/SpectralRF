#ifndef PEAK_DETECTOR_H
#define PEAK_DETECTOR_H

#include <map>
#include "fft_hist.h"
#include <QMap>
#include <utils.h>

using namespace std;

/*
This class looks for peaks inside the FFT history

It will return them in an array
*/

class peak_detector
{
public:
    peak_detector();
    // peak detection
    void SetPeakThreshold(float PeakDetection ){m_PeakDetection = PeakDetection;}
    float GetPeakThreshold(){ return m_PeakDetection;}
    void DetectPeaks();
    void DumpPeaks();
    void SetFFT(FFT_Hist *fft_hist){m_fft_hist = fft_hist;}

    // should these go in here or in the fft helper class?
    float LocalNoiseFloor(float freqlowHz, float freqhighHz, float *stddev);
    float LocalNoiseFloor(int idx,int binwidth, float *stddev, float *tdat);
    float AverageNoiseFloor(float *stddev);
    void DetectChange(); // check for changes in the cf or BW

    // stuff for peak detection
   // int m_noisefloorwidthbins; // this corresponds to a bandwidth
    float m_PeakDetection; // the multiple of the standard deviation for detection
    QMap<int,float> m_Peaks; // the detected peaks
private:
    FFT_Hist *m_fft_hist;

};

#endif // PEAK_DETECTOR_H
