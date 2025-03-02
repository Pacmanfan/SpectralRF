#include "fft_fftw.h"
#include <cstring>
#include <cstdint>
fft_fftw::fft_fftw()
{
    fftsize = 0;
    plancreated = false;
}

fft_fftw::~fft_fftw()
{
    if(plancreated)
    {
        fftwf_destroy_plan(p);
        fftwf_free(in);
        fftwf_free(out);
    }
}

void fft_fftw::PerformFFT(complex<float> *inIQ, float *outFFT, unsigned int numsamples)
{
    if(numsamples != fftsize)
    {
        fftsize = numsamples;
        //changed, change the plan
        if(plancreated)
        {
            fftwf_destroy_plan(p); // crash here?
            fftwf_free(in);
            fftwf_free(out);
        }
        plancreated = true;
        //(re)create the plan
        in = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fftsize);
        out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fftsize);
        p = fftwf_plan_dft_1d(fftsize, in, out, FFTW_FORWARD, FFTW_MEASURE); // make sure we measure for optimal speedups
    }
    //copy the memory
    memcpy(in,inIQ,sizeof(fftwf_complex)*fftsize);
    //execute the fft
    fftwf_execute(p); /* repeat as needed */
    float *co = (float*)out;// get a pointer to the complex<float> out
    // ----- Normalize In-Place-----
    for(unsigned int k=0; k < fftsize * 2; k++ )
    {
        *co  /= fftsize;
        co++; // move to the next
    }
    //compute power and copy to outFFT

    // Amount to scale the FFT.

    const float pwroffset = -50.0;

    const float fScale = 10.0f;
    // Calculate FFT magnitude for the second half of the spectrum using the
    // first half of the I&Q data.
    uint32_t nHalf = fftsize / 2;
    for( uint32_t i=0; i < nHalf; ++i )
    {
        const float& re = ((float *)(out+i))[0];
        const float& im = ((float *)(out+i))[1];//https://github.com/akontsevich/WidgetBox.git

        const uint32_t nIndex = i + nHalf;
        outFFT[nIndex] = fScale * ::log10(::sqrt( re * re + im * im )) + pwroffset;
    }

    // Calculate FFT magnitude for the first half of the spectrum using the
    // second half of the I&Q data.
    for( uint32_t i=nHalf; i < fftsize; ++i )
    {
        const float& re = ((float *)(out+i))[0];
        const float& im = ((float *)(out+i))[1];
        const uint32_t nIndex = i - nHalf;
        outFFT[nIndex] = fScale * ::log10(::sqrt( re * re + im * im )) + pwroffset;

    }
}

