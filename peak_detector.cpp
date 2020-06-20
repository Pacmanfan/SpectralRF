#include "peak_detector.h"

void *PeakDetectorProcessThread(void *dat);

peak_detector::peak_detector()
{
    m_PeakDetection = 3.25f; // a multiple of the standard deviation is the threshold point
    m_Peaks = QMap<int,float>();
}


float peak_detector::AverageNoiseFloor(float *stddev)
{
    float   mean = 0;
    float   sum_of_sq = 0;

    int range = m_fft_hist->GetBinSize();
    float *tdat = m_fft_hist->AvgData();
    for (int i = 0; i < m_fft_hist->GetBinSize(); i++)
    {
        mean += tdat[i];
        sum_of_sq += tdat[i] * tdat[i];
    }
    mean /= range;

    *stddev = sqrt( sum_of_sq/range-mean*mean );
    return mean;
}

float peak_detector::LocalNoiseFloor(int idx,int binwidth, float *stddev, float *tdat)
{
    int lowidx = idx - binwidth;
    int highidx = idx + binwidth;

    if(lowidx < 0 )lowidx = 0;
    if(highidx > m_fft_hist->GetBinSize())
        highidx = m_fft_hist->GetBinSize();

  //  printf("************FFT_Helper::LocalNoiseFloor\r\n");
    float   mean = 0;
    float   sum_of_sq = 0;

    int range = highidx - lowidx;

    for (int i = lowidx; i < highidx; i++) //calculate the mean of the FFT and the sum of squares
    {
        mean += tdat[i];
        sum_of_sq += tdat[i] * tdat[i];
    }
    mean /= range;
    // now calculate the standard deviation
    *stddev = sqrt( sum_of_sq/range-mean*mean );
    return mean;
}

/*
        // Peak detection
        if (m_PeakDetection > 0)
        {
            m_Peaks.clear();

            float   mean = 0;
            float   sum_of_sq = 0;
            for (i = 0; i < n; i++)
            {
                mean += m_fftbuf[i + xmin];
                sum_of_sq += m_fftbuf[i + xmin] * m_fftbuf[i + xmin];
            }
            mean /= n;
            float stdev= sqrt(sum_of_sq / n - mean * mean );

            int lastPeak = -1;
            for (i = 0; i < n; i++)
            {
                //m_PeakDetection times the std over the mean or better than current peak
                float d = (lastPeak == -1) ? (mean - m_PeakDetection * stdev) :
                                           m_fftbuf[lastPeak + xmin];

                if (m_fftbuf[i + xmin] < d)
                    lastPeak=i;

                if (lastPeak != -1 &&
                        (i - lastPeak > PEAK_H_TOLERANCE || i == n-1))
                {
                    m_Peaks.insert(lastPeak + xmin, m_fftbuf[lastPeak + xmin]);
                    painter2.drawEllipse(lastPeak + xmin - 5,
                                         m_fftbuf[lastPeak + xmin] - 5, 10, 10);
                    lastPeak = -1;
                }
            }
        }
*/

void peak_detector::DetectPeaks()
{
    //Peak detection
    m_Peaks.clear(); // clear previous detected peaks

    int numbins = m_fft_hist->GetBinSize();
    float tdat[numbins];
    float *data = m_fft_hist->AvgData();
    float   mean = 0;
    float   stdev = 0;

    m_fft_hist->Lock();

    float sigwidth = 12500;
    int m_noisefloorwidthbins = (sigwidth * SIG_WIDTH_SCALER) / m_fft_hist->GetRBWHz();

    //convert to absolute positive values
    for(int c = 0; c < m_fft_hist->GetBinSize(); c++)
    {
        tdat[c] = fabs(data[c]);
    }
    m_fft_hist->Unlock();

/*
    //calculate the mean of the FFT and the sum of squares
    for (int i = 0; i < numbins; i++)
    {
        mean += tdat[i];
        sum_of_sq += tdat[i] * tdat[i];
    }
    mean /= numbins;
    // now calculate the standard deviation
    stdev = sqrt( sum_of_sq/numbins-mean*mean );
*/
    int lastPeak = -1;
  //  printf("DetectPeaks mean:%f , sumofsq:%f , stddev:%f\r\n",mean,sum_of_sq, stdev );

    for (int bin = 0; bin < numbins; bin++)
    {
        mean = LocalNoiseFloor(bin,m_noisefloorwidthbins,&stdev,tdat);
        //m_PeakDetection times the std over the mean or better than current peak
        float d = (lastPeak == -1) ? (mean - m_PeakDetection * stdev) :  tdat[lastPeak];

        if (tdat[bin] < d)
            lastPeak = bin;

        if (lastPeak != -1 && (bin - lastPeak > PEAK_H_TOLERANCE || bin == numbins - 1))
        {
            m_Peaks.insert(lastPeak, tdat[lastPeak]);
            lastPeak = -1;
        }
    }
   // DumpPeaks();
}

void peak_detector::DumpPeaks()
{
    //iterate through the peaks
    // dump them to stdout
    printf("Number of peaks: %d\r\n", m_Peaks.size());
    for(QMap<int,float>::iterator it = m_Peaks.begin(); it != m_Peaks.end(); ++it)
    {
        printf("Peak at Freq:%f , dBM: %f\r\n",m_fft_hist->GetFreqHz(it.key()), it.value());
    }
}


