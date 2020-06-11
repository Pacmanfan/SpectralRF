#include "sigtuner.h"
#define TUNER_PROCESS_SIZE 10000 // 10k samples at a time are processed

SigTuner::SigTuner()
    : QObject(0)
{
    connect(&_marker,SIGNAL(MarkerChanged(ftmarker*)),this,SLOT(OnMarkerChanged(ftmarker*)));
}

/*
This will index into the stream at the desired start index position and
tune, filter, and resample data.
It will produce num_out_samples and store them in outbuff
*/
long SigTuner::Produce(long startidx, long num_out_samples, complex<float> *outbuff)
{

    static std::complex<float> sindata[TUNER_PROCESS_SIZE* 4]; // storage for the sin data
    static std::complex<float> samples[TUNER_PROCESS_SIZE* 4]; //

    long cursampidx = startidx; // start at the correct spot
    //long total_input_samples_needed = num_out_samples * (1.0 / _resamplerate); // calculate the total number of samples we'll need from the source stream
    long outputindex = 0; // indexing into the output array
    //calculate the number of output samples produced per pass
    double output_samples_per_pass =  _resamplerate * TUNER_PROCESS_SIZE;
    // process TUNER_PROCESS_SIZE samples at a time till done
    long input_samples_this_pass = TUNER_PROCESS_SIZE;
    bool done = false;
    // _resamplerate  = output_rate/ input_rate  // ex: 10Msps / 100 MSPS = .1
    while(!done)
    {
        // check to see if this pass will fulfill all the requested data
        if(outputindex + output_samples_per_pass > num_out_samples)
        {
            //if so, adjust the input_samples_this_pass and output_samples_per_pass
            //and mark this as done after this pass
            output_samples_per_pass = num_out_samples - outputindex;
            input_samples_this_pass = output_samples_per_pass * (1/_resamplerate);
            done = true;
        }

        //get the sine data
        sigsrc.work(input_samples_this_pass,sindata);

        //get the input samples
        _indexer->GetSamples(cursampidx,input_samples_this_pass,samples);
        //increment current sample index
        cursampidx += input_samples_this_pass;
        //shift the signal by the frequency offset to re-tune it
        for(unsigned int c = 0 ; c < input_samples_this_pass; c++)
            samples[c] *= sindata[c];

        //resample and filter the data
        // it gets copied to the outbuff
        unsigned int actualoutputsamples;
        resamp.resample(samples,&outbuff[outputindex],input_samples_this_pass,&actualoutputsamples);

        //increment the index for the output array
        outputindex  += actualoutputsamples;
    }

    return outputindex;
}

/*
 * This awill take an input buffer with num_in_samples and produce an output array of the DDC values
 * This function will return the number of samples in outbuff
 */

long SigTuner::Produce(complex<float> *inbuff, long num_in_samples, complex<float> *outbuff)
{

    static std::complex<float> *sindata = 0;
    static int insz = 0;
    if(insz < num_in_samples)
    {
        insz = num_in_samples;
        if(sindata != 0)
        {
            delete []sindata;
        }
         sindata = new complex<float>[num_in_samples];
    }

    //get the sine data
    sigsrc.work(num_in_samples,sindata);

    //shift the signal by the frequency offset to re-tune it
    for(unsigned int c = 0 ; c < num_in_samples; c++)
        inbuff[c] *= sindata[c];

    //resample and filter the data
    // it gets copied to the outbuff
    unsigned int actualoutputsamples;
    resamp.resample(inbuff,outbuff,num_in_samples,&actualoutputsamples);
    return actualoutputsamples;
}

// this sets up and tears down the tuner every time it's called
void SigTuner::Setup(SampleIndexer *indexer, double BW_Hz, double CF_Hz, int streamid)
{
    _BW_Hz = BW_Hz;
    _CF_Hz = CF_Hz;
    _indexer = indexer;
    _streamid = streamid;

    //set up the resampler
    resamp.set_attenuation(90); // 60
    _resamplerate  = _marker.BW_Hz() / _BW_Hz;
    if(_resamplerate <= 0)
        return;
    if(_lastrate != _resamplerate)
    {
        resamp.set_rate(_resamplerate);
        resamp.set_filter_width(_resamplerate/2.0);
        _lastrate = _resamplerate;
    }


    //now the offset tuning..
    // set up the signal source for offset tuning
    sigsrc.set_sampling_freq(_BW_Hz);
    sigsrc.set_amplitude(1);
    double freqdiff = (_CF_Hz - _marker.CF_Hz());// / si.BandwidthHZ;
    sigsrc.set_frequency(freqdiff);
}

void SigTuner::OnMarkerChanged(ftmarker *mrk)
{

    Q_UNUSED(mrk);
    //set up the resampler
  //  resamp.set_attenuation(60);
    if(_marker.BW_Hz() < 1000)
    {
        return;
    }


    _resamplerate  = _marker.BW_Hz() / _BW_Hz;
    if(_resamplerate <= 0)
        return;

    if(_lastrate != _resamplerate)
    {
        resamp.set_rate(_resamplerate);
        resamp.set_filter_width(_resamplerate/2.0);
        _lastrate = _resamplerate;
    }

    //now the offset tuning..
    // set up the signal source for offset tuning
    sigsrc.set_sampling_freq(_BW_Hz);
    sigsrc.set_amplitude(1);
    double freqdiff = (_CF_Hz - _marker.CF_Hz());// / si.BandwidthHZ;
    sigsrc.set_frequency(freqdiff);


}
