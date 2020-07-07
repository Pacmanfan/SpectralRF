#include "signaldetector.h"
#include <time.h>
#include "math.h"
#include <iostream>
//#include <utils.h>

using std::cout;

static int idgen = 0;

/*
Now, there's a few techniques we can use here to detect signals.

Narrow band signals can be somewhat easily detected by looking for energy peaks - peakdetector
each one of these peaks can be visited to walk fro mthe center outward in bins of the fft
we can detect bandwidth this way for nice, peaked signals pretty easily.

The local noise floor of surrounding the peak of the singal to the upper /lower freqs provide
a basis to detect peaks using the sum of square and standard deviation of the local area
if there are too many local signals, the noise floor may be distorted to be a higher level.

This would be reflected through in the max standard devation. This might be use to detect when to change
the detection approach from local noise floor to absolute noise floor.

Maybe the abs noise floor can be mixed with a ration to determine proper local noise floor

We should use a different method for detecting wide band signals. We can walk the fft bins from left to right -
low to high, and see when the signal triggers the noise floor and thresholds

Process:
-IQ to fft
-Peak detection on fft bins
For each Detection:
    detect narrow band signal from walking sides to abs or wide noise floor

Use FFT Bin map to mark off sections

Detect wide signals first and add to map
detect narrow signals

Try to re-detect or re-identify signals previously seen and update count and last seen.
look for signals that appear periodiacally,

age off old signals

We're looking for signals in the X->Y Frequency plane just as much as we're looking for signals progressing through the Y Time plane


*/

void *SignalDetectorProcessThread(void *dat);

SignalDescriptorEntry::SignalDescriptorEntry()
{
    lastseen = clock();
    count = 1;
    threshhigh = -150;
    threshlow = 0;
    classcount = 0; // how man times has this been classified / verified
    classified = false; // hjas this been classified
    extractedcount = 0;

}

void SignalDescriptorEntry::Dump()
{
    cout << "ID:" << identifier << " CF:" << centerfreqHz << " BW:" << bwHz << "\r\n";
}

void SignalDescriptorEntry::Merge(SignalDescriptorEntry *sde)
{

}

//SignalDetector::SignalDetector(QObject *parent) :
//    QObject(parent)
SignalDetector::SignalDetector(int binsize)
{
    m_maplen = 0;
    m_binsize = binsize;
    pfft = nullptr;
    m_map = new char[m_binsize];
    m_sdemap = new SignalDescriptorEntry*[m_binsize];
    for(int c =0; c < m_binsize; c++)
        m_sdemap[c] = nullptr;

}

/*
This function is for detecting chnages in the bandwidth or center frequency
*/
void SignalDetector::DetectChange()
{
    if(m_curCF != pfft->GetCenterFreqHz() || m_curBW != pfft->GetBWHz())  //for detecting changes in the cf or bw
    {
        //reset detection map
        m_curCF = pfft->GetCenterFreqHz();
        m_curBW =  pfft->GetBWHz();
        m_entries.clear(); // remove previous entries
    }
}


/*
We should have multiple levels of signal detection
The first level takes a look at the instantaneous results in the peak detect, and converts them somewhat blindly into potential
signals (PotentialSignalList)
*/

void SignalDetector::Update(float peaksensistivity)
{
    DetectChange();
    pfft->Lock();
    InitMap(); // initialize the fft map;
    //walk the frequencies from low to high
    SignalDescriptorEntry *sd;
    int cntridx;
    float cntrfreqHz;
   // float stddev[pfft->GetBinSize()];
    pfft->CalcNoiseFloor(0);
    float *localnoisefloor = pfft->GetNoiseFloor();
    // iterate through all the detected peaks
    QMap<int,float> peaks = pfft->DetectPeaks(peaksensistivity);
    for(QMap<int,float>::iterator it = peaks.begin(); it != peaks.end(); ++it)
    {
        cntridx = it.key(); // the the center bin index
        cntrfreqHz = pfft->GetFreqHz(cntridx); // get the frequency at that bin

        sd = new SignalDescriptorEntry();
        sd->centerfreqHz = cntrfreqHz;//
        //starting at the center frequency, walk the index to the left / right to determine bandwidth
        //should we impose a max BW, or just 'let it happen'?
        // we should figure out the RBW
        // this gets into the question about how wide can our engery detector reliably work.
        bool done = false;
        bool valid = true;
        int leftidx = cntridx;
        int rightidx = cntridx;
        float curdbm;
        float sf,ef,sb,eb,tf;

        float rbw = pfft->GetRBWHz();// get resolution bandwidth
        sd->localnoisefloor = localnoisefloor[cntridx];
         // get the power at the center
        float centerdbm = pfft->AvgData()[cntridx];
        // find the range of the signal based on the local noise floor

        float dbmrange = centerdbm - localnoisefloor[cntridx];
        float lowthreshold = ((dbmrange / 3) + localnoisefloor[cntridx]);

        // walk the index to the left (lower freq) until we pass the threshold
        while(!done)
        {
            curdbm = pfft->AvgData()[leftidx]; // get the value
      //      printf("Current Left DBm %f at idx %d\r\n",curdbm,leftidx);
            if(curdbm < lowthreshold)
            {
                done = true;
                sd->startf = pfft->GetFreqHz(leftidx);
         //       printf("Found New Left IDX : %d, freq: %f\r\n",leftidx,sd->startf);
                // interpolate the frequency
                sf = pfft->GetFreqHz(leftidx);
                ef = pfft->GetFreqHz(leftidx + 1);
                sb = pfft->AvgData()[leftidx];
                eb = pfft->AvgData()[leftidx + 1];
                tf = sf + (ef - sf) * ((lowthreshold - sb)/(eb-sb));
         //       printf("Interpolated Left Freq = %f\r\n",tf);
                sd->startf = tf;

            }else
            {
                leftidx--; // move to the left
                if(leftidx <= 0)
                {
                    //cut off...
             //       printf("!!!!!!!!!Left Cut OFF\r\n");
                    done = true;
                    valid = false;
                }
            }
        }
        done = false;
        while(!done)
        {
            curdbm = pfft->AvgData()[rightidx]; // get the value
            if(curdbm < lowthreshold)
            {
                done = true;
                //sd->endf = pfft->GetFreqHz(rightidx);
        //        printf("Found New Right IDX : %d, Freq = %f \r\n",rightidx,sd->endf);
                sf = pfft->GetFreqHz(rightidx - 1);
                ef = pfft->GetFreqHz(rightidx);
                sb = pfft->AvgData()[rightidx - 1];
                eb = pfft->AvgData()[rightidx];
                tf = sf + (ef - sf) * ((lowthreshold - sb)/(eb-sb));
         //       printf("Interpolated Right Freq = %f\r\n",tf);
                sd->endf = tf;
            }else
            {
                rightidx++; // move to the right
                if(rightidx >= pfft->GetBinSize() - 1)
                {
                    done = true;
           //         printf("!!!!!!!!!Right Cut OFF\r\n");
                    valid = false;
                }
            }
        }
        sd->bwHz = sd->endf - sd->startf;

        if(sd->bwHz < MIN_SIGNAL_WIDTH) // why are we getting negative values?
        {
            valid = false;
        }        

        if(valid)
        {
            AddDetection(sd);
        }else
        {
            delete sd;
        }
    }
    UpdateMinMax();
    pfft->Unlock();
}

// monitor and determine high / low thresholds
void SignalDetector::UpdateMinMax()
{
    for(int c = 0 ;c < m_entries.size(); c++)
    {
        SignalDescriptorEntry *sd = m_entries.at(c);
        int idx = pfft->GetBinIndex( sd->centerfreqHz);
        if(idx != -1)
        {
            // look at the dBM value at the center
            float dbm = pfft->AvgData()[idx];
            if(dbm > sd->threshhigh) sd->threshhigh = dbm;
            if(dbm < sd->threshlow) sd->threshlow = dbm;
        }
    }
}

bool CheckSimilarFreq(float f1Hz,float f2Hz, float tolerance)
{

    if(f2Hz >= f1Hz - tolerance  &&
       f2Hz <= f1Hz + tolerance)
    {
        return true;
    }
    return false;
}

void SignalDetector::AddDetection(SignalDescriptorEntry *entry)
{
    if(CheckMap(entry) == false)//no entry in the map yet, enter one.
    {        
        UpdateMap(entry);
        m_entries.append(entry);
        entry->identifier = idgen++;
    }else
    {
        SignalDescriptorEntry *found = CheckMapEntry(entry); //find the previous entry if any
        if(found != nullptr)
        {
            found->count ++; // increment it's usage count
            found->lastseen = clock(); // update the last time it was seen
        }
    }
}

SignalDetector::~SignalDetector()
{
    delete []m_map;
    delete []m_sdemap;
    m_entries.clear();
}

void SignalDetector::Dump()
{
    cout << "SignalDetector::Dump\r\n";
    for (int c = 0; c < m_entries.size(); c++)
    {
        SignalDescriptorEntry * sde = m_entries[c];
        sde->Dump();
    }
}

bool SignalDetector::IsActive(SignalDescriptorEntry *sd)
{
    int idx = pfft->GetBinIndex( sd->centerfreqHz);
    if(idx != -1)
    {
        // look at the dBM value at the center
        float dbm = pfft->AvgData()[idx];
        float range = sd->threshhigh - sd->threshlow;
        if(dbm > (sd->threshlow + (range * .66)))
            return true;
    }
    return false;
}



void SignalDetector::InitMap()
{
    //  create the map if needed
    if(m_maplen != pfft->GetBinSize())
    {
        m_maplen = pfft->GetBinSize();
    }
    //clear old data
    for(int c =0; c < m_binsize; c++)
        m_map[c] = 0;

    // now iterate through all entries and place them on the map.

    for(int c = 0 ;c < m_entries.size(); c++)
    {
        SignalDescriptorEntry *sd = m_entries.at(c);
        // we can just add in the entries here without checking them, because
        // they were checked when they first got on the list
        UpdateMap(sd);
    }
}

// returns true if any portion of the index range is taken
bool SignalDetector::CheckMap(SignalDescriptorEntry *sde)
{
    // convert the bandwidth to start and stop indexes

    int flow,fhigh;
    //get the indexes
    flow = pfft->GetBinIndex(sde->startf);
    fhigh = pfft->GetBinIndex(sde->endf);
    if(flow == -1) return true;
    if(fhigh == -1) return true;

    for(int c = flow; c < fhigh; c++)
    {
        if(m_map[c] != 0)
            return true;
    }
    return false; // no entries
}

SignalDescriptorEntry *SignalDetector::CheckMapEntry(SignalDescriptorEntry *sde)
{
    int fcenter;
    //get the indexes
    fcenter = pfft->GetBinIndex(sde->centerfreqHz);
    if(fcenter == -1) return nullptr;
    return m_sdemap[fcenter];
}

void SignalDetector::UpdateMap(SignalDescriptorEntry *sde)
{
    int flow,fhigh;
    //get the indexes
    flow = pfft->GetBinIndex(sde->startf);
    fhigh = pfft->GetBinIndex(sde->endf);
    if(flow == -1) return;
    if(fhigh == -1) return;

    for(int c = flow; c < fhigh; c++)
    {
        m_map[c] = 1; //mark as used
        m_sdemap[c] = sde;
    }
    return;  // no entries
}

