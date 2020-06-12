#include "sweeper.h"

void *threadfn(void *param);

long long GetTimeuS()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

Sweeper::Sweeper(QObject *parent) : QObject(parent)
{
    m_radio = nullptr;
    m_sweeping = false;
    m_ffthist = new FFT_Hist();
    SetCorrectIQDrop(true);
    m_settletime_uS = DEFAULT_SETTLE_TIME;
}
void Sweeper::StartSweep(double flow,double fhigh, int nBinsPerFFT, int overlappercent)
{
    if(m_radio == nullptr)
        return;
    m_freq_low = flow; // set the low and high frequencies for this scan
    m_freq_high = fhigh;
    m_nBinsPerFFT = nBinsPerFFT;
    m_overlappercent = overlappercent;
    m_sweeping = true;
    pthread_create(&rst, NULL, threadfn, this);
}

void Sweeper::StopSweep()
{
    m_sweeping = false;
    pthread_join(rst,0);
}

void Sweeper::Sweep()
{
    std::complex<float> buff[m_nBinsPerFFT];

    void *buffs[] = {buff};
    int flags;
    long long time_ns;
    double freq_low,freq_high; // we're putting this into a local variable so it can change later
    freq_low = m_freq_low;
    freq_high = m_freq_high;

    double sweep_bw = freq_high - freq_low; // the entire bw that we're sweeping
    double sweep_cf = freq_low + (sweep_bw / 2.0); // the center frequency of the bandwidth we're sweepinh
    double fstart = freq_low + (m_radio->m_BW_Hz / 2.0); // the start frequency
    double rbw = m_radio->m_BW_Hz / m_nBinsPerFFT; //resolution bandwidth
    int totalsweepbins = (int)(sweep_bw / rbw); // total number of bins we've pre-allocated
    double fcurrent = 0.0,fold = 0.0; // current and last frequency
    double fcurlow,fcurhigh;
    double overlap = ((double)m_overlappercent)/100.0; // the % overlap on each side of the fft data
    // set the start freq
    fcurrent = fstart;
    //now, initialize the fft_hist to allocate memory for all this
    m_ffthist->Reset(totalsweepbins);
    SweepDataLine *sweepline = new SweepDataLine(totalsweepbins);
    fft_fftw *fftw = new fft_fftw(); // for calculating the FFT
    m_radio->sdr->activateStream( m_radio->rx_stream, 0, 0, 0);
    int curnumbins = m_nBinsPerFFT; // the current number of bins we're using
    int ret = 0;
    int idx = 0;
    while(m_sweeping)
    {
        //calculate the current low freq
        fcurlow = fcurrent - (m_radio->m_BW_Hz / 2.0);
        if(fold != fcurrent) // check to see if frequency is changing
        {
            m_radio->sdr->setFrequency(SOAPY_SDR_RX, 0,fcurrent);//move to the new center frequency            
            EatSamples(); //wait a certain 'settle time' and continue eating samples
        }

        fold = fcurrent; //save the old freq

        ret = m_radio->sdr->readStream( m_radio->rx_stream, buffs, curnumbins, flags, time_ns);//get the IQ data

        if(ret < 0)
            continue;

        //calculate the FFT for this segment
       // int idx = (int)((fcurlow - freq_low )/ rbw); // calculate the bin offset

        sweepline->m_timestamp = time_ns;
        fftw->PerformFFT(buff,&sweepline->m_data[idx],curnumbins);
        if(m_correctIQDrop) // get rid of the center frequency dropout by averaging the 2 adjacent freqs
        {
            int hf = (curnumbins >> 1); // find the center bin
            sweepline->m_data[hf + idx] = ( sweepline->m_data[hf + idx - 1] + sweepline->m_data[hf + idx + 1])*.5;
        }
        idx += curnumbins;
        //increment the frequency
        double nextfreq = fcurrent + m_radio->m_BW_Hz;
        //now, some checks
        fcurlow = nextfreq - (m_radio->m_BW_Hz / 2.0);
        fcurhigh = nextfreq + (m_radio->m_BW_Hz / 2.0);
        //check to see if we're done with the sweep        
        if(fcurlow >= freq_high)
        {
            fcurrent = fstart;// back to the start
            // set the fft results
            m_ffthist->AddData(sweepline->m_data,totalsweepbins,sweep_cf,sweep_bw);            
            emit(SweepCompleted());//signal line data is ready
            idx = 0; //reset the index
        }
        else if (fcurhigh > freq_high)
        { //need to back it off a little
            fcurrent = freq_high - (m_radio->m_BW_Hz / 2.0);
        }
        else
        {
            fcurrent = nextfreq;
        }
    }

    delete sweepline;
    delete fftw;
}

void Sweeper::SetViewRange(double lower, double upper)
{
    m_viewfreqlow = lower;
    m_viewfreqhigh = upper;
}

/*
Eat sample for the duration of the 'settle time'
*/
void Sweeper::EatSamples()
{
    int sz = 16384;
    long long startsettletime = GetTimeuS(); // mark the start of the settle time
    static std::complex<float> buff[16384];
    void *buffs[] = {buff};
    int flags;

    //long samples_to_eat; // using the bandwidth and the delay tie, calculate the number of sampels to eat
    double samples_to_eat;

    long long time_ns;
    bool done = false;
    int ret = 0;
    samples_to_eat = m_radio->m_BW_Hz / 100000; // convert to samples per uS
    samples_to_eat *= m_settletime_uS;
    double sampleseaten = 0;

    do
    {
        //read samples
        ret = m_radio->sdr->readStream( m_radio->rx_stream, buffs, sz, flags, time_ns,0); //no timeout
        if(time_ns !=0)
        {
            int a = 100;
        }
        /*
        if(ret < 0)
        {
            switch(ret)
            {
                case -1:break;
                case -2:break;
                case -3:break;
                case -4:
                    continue;  // timeout
                    break;
            }
        }
        else if (ret != sz)
        {
            // ret is > 0, but less than request, probably safe to continue
          // done = true;
        }
*/
        if(ret > 0)
        {
            sampleseaten += ret;
            if(sampleseaten >= samples_to_eat)
                done = true;
        }
        /*
        long long timenow = GetTimeuS();
        if(timenow < startsettletime) // rollover
        {
            swap(timenow,startsettletime);
        }
        if((timenow - startsettletime) > m_settletime_uS)
            done = true;
*/

        //done = true;
    }while(!done);
}


void *threadfn(void *param)
{
    Sweeper *swp = (Sweeper *)param;
    swp->Sweep();
    return nullptr;
}

SweepDataLine::SweepDataLine(int numbins)
{
    m_timestamp=0;// the timestamp of when this sweep was completed
    m_data = new float[numbins];
    m_numbins = numbins; // total number of bins across the sweepline
}
