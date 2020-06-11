#ifndef SWEEPER_H
#define SWEEPER_H

#include <QObject>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>

#include <string>	// std::string
#include <vector>	// std::vector<...>
#include <map>		// std::map< ... , ... >
#include <pthread.h>
#include <iostream>
#include <fft_hist.h>
#include <fft_fftw.h>
#include <sdr_device.h>

/*
This class holds FFT data for a single line swept from low to high
so, for a sweep of a 100Mhz bandwidth:
flow = 800Mhz
fhigh = 900Mhz
sweep_bw = (900 - 800) = 100Mhz
sdr_bw = 20Mhz
fstart = flow + (sdr_bw / 2)
fstart = 800 + ( 20 / 2) = 810

choosing a bin size of 4096 per fft:
rbw = sdr_bw / binsize
rbw = 20Mhz / 4096 = 4882.8125 Hz
total bins needed for sweep bandwidth =
tbins = sweep_bw / rbw
tbins = 100Mhz / 4882.8125 = 20480 bins
*/

#define DEFAULT_SETTLE_TIME 5000

struct SweepDataLine
{
    SweepDataLine(int numbins);
    long long m_timestamp;// the timestamp of when this sweep was completed in nanoseconds
    float *m_data;
    int m_numbins; // total number of bins across the sweepline
};

class Sweeper : public QObject
{
    Q_OBJECT
public:
    explicit Sweeper(QObject *parent = nullptr);
    SDR_Device *m_radio;

    void SetRadio(SDR_Device *radio){m_radio = radio;}
    void StartSweep(double flow,double fhigh, int nBinsPerFFT, int overlappercent);
    void StopSweep();
    void Sweep();
    bool IsSweeping(){return m_sweeping;}
    void SetCorrectIQDrop(bool val){m_correctIQDrop = val;}
    bool GetCorrectIQDrop(){return m_correctIQDrop;}
    void SetViewRange(double lower, double upper);
    void EatSamples();
    double m_freq_low,m_freq_high; // the low and high freqs that we're scanning across
    double m_viewfreqlow,m_viewfreqhigh;// where the user is actually looking at in Mhz
    int m_nBinsPerFFT; // the FFT resolution per segment (the SDR bandwidth, not the sweepline bins)
    int m_overlappercent;
    FFT_Hist *m_ffthist;
    bool m_correctIQDrop;
    long long m_settletime_uS; // the time to allow to settle after changing frequencies
private:
    bool m_sweeping;
    pthread_t rst; //  thread
    fft_fftw fft; // This calculates the FFT from the IQ timeseries.

signals:
    void SweepCompleted(); // a single sweep was completed

public slots:
};

#endif // SWEEPER_H
