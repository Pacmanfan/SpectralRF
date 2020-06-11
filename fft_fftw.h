#ifndef FFT_CALC2_H
#define FFT_CALC2_H
#include <complex>
#include <fftw3.h>

using namespace std;

class fft_fftw
{
public:
    fft_fftw();
    ~fft_fftw();
    void PerformFFT(complex<float> *inIQ, float *outFFT, unsigned int numsamples);
private:
    unsigned int fftsize;
    fftwf_plan p;
    bool plancreated;
    fftwf_complex *in, *out;
};

#endif // FFT_CALC2_H
