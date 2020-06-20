#include "sweeper.h"
#include "sweepfile.h"
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
    m_fftw = new fft_fftw(); // for calculating the FFT
    m_sweepline = nullptr;
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
   // m_ffthist->Reset(m_nBinsPerFFT);
    double sweep_bw = m_freq_high - m_freq_low; // the entire bw that we're sweeping
    double rbw = m_radio->m_BW_Hz / m_nBinsPerFFT; //resolution bandwidth
    int totalsweepbins = (int)(sweep_bw / rbw); // total number of bins we've pre-allocated
    m_ffthist->Reset(totalsweepbins);
    m_sweepline = new SweepDataLine(totalsweepbins);

    pthread_create(&rst, NULL, threadfn, this);
}

void Sweeper::StopSweep()
{
    m_sweeping = false;
    pthread_join(rst,0);
    delete m_sweepline;
    m_sweepline = nullptr;

}

void Sweeper::Sweep()
{
    std::complex<float> buff_IQ[m_nBinsPerFFT]; // buffer to hold the IQ samples
    float buff_FFT[m_nBinsPerFFT]; // buffer to hold the FFT data
    void *buffs[] = {buff_IQ};
    int flags;
    long long time_ns;
    double freq_low,freq_high; // we're putting this into a local variable so it can change later
    freq_low = m_freq_low;
    freq_high = m_freq_high;

    double sweep_bw = freq_high - freq_low; // the entire bw that we're sweeping
    double sweep_cf = freq_low + (sweep_bw / 2.0); // the center frequency of the bandwidth we're sweeping
    double half_bw = (m_radio->m_BW_Hz / 2.0); // half the bandwidth
    double rbw = m_radio->m_BW_Hz / m_nBinsPerFFT; //resolution bandwidth
    int totalsweepbins = (int)(sweep_bw / rbw); // total number of bins we've pre-allocated
    double fcurrent = 0.0,fold = 0.0; // current and last frequency
    double fcurlow,fcurhigh; // the current low/high frequency of this segment
    double overlap = (1.0 - ((double)m_overlappercent)/100.0); // compute scaler
    // set the start freq
    //now, initialize the fft_hist to allocate memory for all this
    //m_ffthist->Reset(totalsweepbins);


    m_radio->sdr->activateStream( m_radio->rx_stream, 0, 0, 0);
    int ret = 0;
    int idx = 0;
    int adj_numbin = m_nBinsPerFFT * overlap ; // adjusted number of bins
    int bins_diff = m_nBinsPerFFT - adj_numbin; // the difference between the full and adjusted bins
    int bins_diff_half = bins_diff / 2; // and half that
    double fstart = freq_low + (half_bw * overlap); // the start frequency // is this right?
    fcurrent = fstart;

    while(m_sweeping)
    {
        //calculate the current low freq
        fcurlow = fcurrent - (half_bw * overlap);
        if(fold != fcurrent) // check to see if frequency is changing
        {
            m_radio->sdr->setFrequency(SOAPY_SDR_RX, 0,fcurrent);//move to the new center frequency            
            EatSamples(); //wait a certain 'settle time' and continue eating samples
        }

        fold = fcurrent; //save the old freq
        ret = m_radio->sdr->readStream( m_radio->rx_stream, buffs, m_nBinsPerFFT, flags, time_ns);//get the IQ data
        if(ret == -1)
        {
           // printf("Readstream returned -1 \r\n");
            continue;
        }

        if(ret < 0)continue; // keep reading on error

        m_sweepline->m_timestamp = time_ns;
        m_fftw->PerformFFT(buff_IQ,buff_FFT,m_nBinsPerFFT); // compute the FFT if this IQ series

        if(m_correctIQDrop) // get rid of the center frequency dropout by averaging the 2 adjacent freqs
        {
            int hf = (m_nBinsPerFFT >> 1); // find the center bin
            buff_FFT[hf] = ( buff_FFT[hf - 1] + buff_FFT[hf + 1]) * 0.5;
        }

        int numcopy = adj_numbin;
        if(idx + adj_numbin > totalsweepbins)
        {
            numcopy = totalsweepbins - idx;
        }
        memcpy(&m_sweepline->m_data[idx],&buff_FFT[bins_diff_half],numcopy * sizeof(float)); // original

        idx += adj_numbin;
        //increment the frequency
        double nextfreq = fcurrent + (m_radio->m_BW_Hz * overlap);
        //now, some checks

        fcurlow = nextfreq - (half_bw * overlap);
        fcurhigh = nextfreq + (half_bw * overlap);
        //check to see if we're done with the sweep        
        if(fcurlow >= freq_high)
        {
            fcurrent = fstart;// back to the start
            // set the fft results
            m_ffthist->AddData(m_sweepline->m_data,totalsweepbins,sweep_cf,sweep_bw,m_sweepline->m_timestamp);
            emit(SweepCompleted());//signal line data is ready
            idx = 0; //reset the index
        }
        else if (fcurhigh > freq_high) // if not done with sweep, check to see if high freq is past the end freq.
        {
            fcurrent = freq_high - (half_bw * overlap); //need to back it off a little
        }
        else
        {
            fcurrent = nextfreq;
        }
    }

    //delete m_sweepline;

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
    static std::complex<float> buff[16384];
    void *buffs[] = {buff};
    int flags;
    double samples_to_eat;
    long long time_ns;
    bool done = false;
    int ret = 0;
    samples_to_eat = m_radio->m_BW_Hz / 100000; // convert to samples per uS
    samples_to_eat *= m_settletime_uS; // calculate the number of samples to eat for the duration
    double sampleseaten = 0;

    do
    {
        //read samples
        ret = m_radio->sdr->readStream( m_radio->rx_stream, buffs, sz, flags, time_ns,0); //no timeout
        if(ret == -1)
        {
           // printf("Readstream returned -1 \r\n");
            continue;
        }
        if(time_ns !=0)
        {
            int a = 100;
        }

        if(ret > 0)
        {
            sampleseaten += ret;
            if(sampleseaten >= samples_to_eat)
                done = true;
        }

    }while(!done);
}


void *threadfn(void *param)
{
    Sweeper *swp = (Sweeper *)param;
    swp->Sweep();
    return nullptr;
}
