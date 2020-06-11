#ifndef SIGNALTUNER_H
#define SIGNALTUNER_H

#include <signalsource.h> // signal source for tuning
#include <resampler_arb.h> // complex to complex resampler / filter
#include <ftmarker.h>
#include <sampleindexer.h>
#include <QObject>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

/*
This signal tuner class does a few things
it has a signal source, such as a sample indexer
it can do a DDC operation such as a tuning to a center frequency,
Filter, and resample the data
it produces data on demand

The output of this can be fed to other stages such as demodulators and decoders

Should I subclass from a FTMarker? Or simply have an FTMarker that describes the target?

Should I use a sample indexer? Or should I use a file reader and simply index to the right spot?

We need a descriptor for the inpuit stream, at least CF and BW

This should be modified to work with generic IQ data, not just a SampleIndexer
*/

class SigTuner : public QObject
{
    Q_OBJECT
public:
    SigTuner();
    long Produce(long startidx, long num_samples, complex<float> *outbuff);

    long Produce(complex<float> *inbuff, long num_in_samples, complex<float> *outbuff);

    void Setup(SampleIndexer *indexer, double BW_Hz, double CF_Hz, int streamid);
    ftmarker *Marker(){return &_marker;}
    double ResampleRate(){return _resamplerate;} // output / input rate
    double SourceSPS(){return _indexer->BW_Hz();}
    SampleIndexer *Indexer(){return _indexer;}
    void setPhase(float phase){sigsrc.setPhase(phase);}
    float getPhase(){return sigsrc.getPhase();}
private:
    double _resamplerate,_lastrate; // the rate at which we're re-sampling
    int _streamid; // When we're using a QVRT file with multiple streams, this indicates the source stream
    SignalSource sigsrc; // Sine wave signal source for the resampler for offset tuning
    SampleIndexer *_indexer; //
    ftmarker _marker; // the marker that describes where this is tuned, it should be relative to the main CF and BW
    resampler_arb resamp; // the re-sampler we're using
    double _CF_Hz,_BW_Hz; // the center frequency and bandwidth of the source stream
    void Lock(){m_mutex.lock();}
    void Unlock(){m_mutex.unlock();}
    boost::mutex m_mutex; // for locking the tables to avoind threading issues
public slots:
    void OnMarkerChanged(ftmarker *mrk);
};

#endif // SIGNALTUNER_H
