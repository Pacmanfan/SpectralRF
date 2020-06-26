#ifndef SIGNALDETECTOR_H
#define SIGNALDETECTOR_H

#include <QObject>
#include "fft_hist.h"
#include <QVector>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <ftmarker.h>
#include <peak_detector.h>

#define MIN_SIGNAL_WIDTH 5000 // the minimum size a signal

class SignalDescriptorEntry
{
public:
    float bwHz; // the estimated bandwidth of the signal
    float centerfreqHz; // the estimated cf
    float startf; //start freq in hz
    float endf; // end freq in hz
    int count; // how many times has this been seen?
    long lastseen;// timestamp

    float localnoisefloor;
    float floorwidth;// how wide (in hz) was this noise floor calculated
    float threshhigh;
    float threshlow;

    // we need some sort of flag or timer that indicates when we tried to classify this signal last.
    int classcount;// How many times have we attempted classification?
    bool classified; // has this been classified?
    int extractedcount; // how many times has this signal been extracted for classification?
    int identifier;
    SignalDescriptorEntry();
    void Dump(); // single line
    bool Save();
    bool Load();
    void Merge(SignalDescriptorEntry *sde); // merge the 2 together for more accurate BW

};

/*
This is a signal detector class, it will iterate through the FFT data fed into it
and output a list of detected signals,
-A more advanced version will also look at previous identified signals to refine estimates
This Signal detector will periodically run in it's own thread to produce results.
The update rate of this can be set using the SetUpdateRateHz function
*/


class SignalDetector
{
public:
    SignalDetector(int binsize);
    ~SignalDetector();
    void SetFFTSource(FFT_Hist * fft){pfft = fft;}
    FFT_Hist * GetFFTSource(){return pfft;}
    void SetPeakDetector(peak_detector * peakdetector){m_peakdetector = peakdetector;}
    void Update();
    void UpdateMinMax(); // monitor and determine high / low thresholds
    void DetectChange();
    void Clear(){m_entries.clear();}
    void Dump();
    bool IsActive(SignalDescriptorEntry *sd);

    void Lock(){m_mutex.lock();}
    void Unlock(){m_mutex.unlock();}
    void UpdateMap(ftmarker *marker); // pre-populate markers on the map
    QVector<SignalDescriptorEntry *> m_entries;

private:
    char *m_map;
    SignalDescriptorEntry **m_sdemap;
    int m_maplen;
    int m_binsize;
    FFT_Hist * pfft;//
    peak_detector * m_peakdetector;
    boost::mutex m_mutex;
    float m_curCF, m_curBW; //for detecting changes in the cf or bw

    void AddDetection(SignalDescriptorEntry *entry);
    void InitMap();
    bool CheckMap(SignalDescriptorEntry *sde);
    SignalDescriptorEntry *CheckMapEntry(SignalDescriptorEntry *sde);
    void UpdateMap(SignalDescriptorEntry *sde);

};

#endif // SIGNALDETECTOR_H
