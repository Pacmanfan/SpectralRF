#include "ftmarker.h"
#include <algorithm>

static int _markercnt = 0;

ftmarker::ftmarker(QObject *parent)
{
    Q_UNUSED(parent);
    setFreqLowHz(-1);
    setFreqHighHz(-1);
    setStartTime_S(-1);
    setEndTime_S(-1);
    setName("Marker_" + QString::number(_markercnt++));
    setTags("");
    m_HasStartTime = true;
    m_HasEndTime = true;
    m_surpress = false;
}

double ftmarker::CF_Hz()
{
    return ((m_FreqHighHz - m_FreqLowHz) /2) + m_FreqLowHz;
}

double ftmarker::CF_MHz()
{
    return (((m_FreqHighHz - m_FreqLowHz) /2) + m_FreqLowHz) / 1000000.0;
}


void ftmarker::setCF_Hz(double cf)
{
    double bw = BW_Hz();
    SurpressSignal();
    setFreqLowHz(cf - (bw/2));
    setFreqHighHz(cf + (bw/2));
    RestoreSignal(true);
}


void ftmarker::setCF_MHz(double cf)
{
    double bw = BW_MHz();
    SurpressSignal();
    setFreqLowMHz(cf - (bw/2));
    setFreqHighMHz(cf + (bw/2));
    RestoreSignal(true);
}

double ftmarker::BW_Hz()
{
    return (m_FreqHighHz - m_FreqLowHz);
}

double ftmarker::BW_MHz()
{
    return (m_FreqHighHz - m_FreqLowHz) / 1000000.0;
}

void ftmarker::setBW_Hz(double BW)
{
    double CF = CF_Hz();
    SurpressSignal();
    setFreqLowHz(CF - (BW/2));
    setFreqHighHz(CF + (BW/2));
    RestoreSignal(true);
}

void ftmarker::setBW_MHz(double BW)
{
    double CF = CF_MHz();
    SurpressSignal();
    setFreqLowMHz(CF - (BW/2));
    setFreqHighMHz(CF + (BW/2));
    RestoreSignal(true);
}


double ftmarker::FreqLowHz() const
{
    return m_FreqLowHz;
}


double ftmarker::FreqLowMHz() const
{
    return m_FreqLowHz / 1000000.0;
}

void ftmarker::setFreqLowHz(double FreqLowHz)
{
    m_FreqLowHz = FreqLowHz;
    Signal();
}

void ftmarker::setFreqLowMHz(double FreqLowMHz)
{
    m_FreqLowHz = FreqLowMHz * 1000000.0;
    Signal();
}


double ftmarker::FreqHighHz() const
{
    return m_FreqHighHz;
}

double ftmarker::FreqHighMHz() const
{
    return m_FreqHighHz/1000000.0;
}

void ftmarker::setFreqHighHz(double FreqHighHz)
{
    m_FreqHighHz = FreqHighHz;
    Signal();
}

void ftmarker::setFreqHighMHz(double FreqHighMHz)
{
    m_FreqHighHz = FreqHighMHz * 1000000;
    Signal();
}

double ftmarker::StartTime_S() const
{
    return m_StartTimeS;
}

void ftmarker::setStartTime_S(double StartTimeS)
{
    m_StartTimeS = StartTimeS;
    Signal();
}

double ftmarker::EndTime_S() const
{
    return m_EndTimeS;
}

void ftmarker::setEndTime_S(double EndTimeS)
{
    m_EndTimeS = EndTimeS;
    Signal();
}

void ftmarker::CopyFrom(ftmarker *ftm)
{
    m_FreqLowHz = ftm-> m_FreqLowHz;
    m_FreqHighHz = ftm->m_FreqHighHz;
    m_StartTimeS = ftm->m_StartTimeS;
    m_EndTimeS = ftm->m_EndTimeS;
    m_Name = ftm->m_Name;
    m_Tags = ftm->m_Tags;
    m_HasEndTime = ftm->m_HasEndTime;
    m_HasStartTime = ftm->m_HasStartTime;
}

void ftmarker::Load(QSettings *settings)
{
    m_FreqLowHz = settings->value("FreqLowHz",0).toDouble();
    m_FreqHighHz = settings->value("FreqHighHz",0).toDouble();
    m_StartTimeS = settings->value("StartTimeS",0).toDouble();
    m_EndTimeS = settings->value("EndTimeS",0).toDouble();
    m_Name = settings->value("Name",m_Name).toString();
    m_Tags = settings->value("Tags",m_Tags).toString();
    m_HasEndTime = settings->value("HasEndTime",true).toBool();
    m_HasStartTime = settings->value("HasStartTime",true).toBool();
    Signal();

}


void ftmarker::Save(QSettings *settings)
{
    settings->setValue("FreqLowHz",m_FreqLowHz);
    settings->setValue("FreqHighHz",m_FreqHighHz);
    settings->setValue("StartTimeS",m_StartTimeS);
    settings->setValue("EndTimeS",m_EndTimeS);
    settings->setValue("Name",m_Name);
    settings->setValue("HasEndTime",m_HasEndTime);
    settings->setValue("HasStartTime",m_HasStartTime);
}

QString ftmarker::Name() const
{
    return m_Name;
}

void ftmarker::setName(const QString &name)
{
    m_Name = name;
    Signal();
}

QString ftmarker::Tags() const
{
    return m_Tags;
}

void ftmarker::setTags(const QString &Tags)
{
    m_Tags = Tags;
    Signal();
}

void ftmarker::SurpressSignal()
{
    m_surpress = true;
}

void ftmarker::RestoreSignal(bool signalnow)
{
    m_surpress = false;
    if(signalnow == true)
        Signal();
}

void ftmarker::Signal()
{
    if(!m_surpress)
        emit(MarkerChanged(this));
}

void freq_markers::Load(QSettings *settings)
{
    int nentries;
    nentries = settings->beginReadArray("freq_markers");
    for(int c= 0; c< nentries; c++)
    {
        settings->setArrayIndex(c);
        ftmarker *fm = new ftmarker();
        fm->Load(settings);
        m_markers.push_back(fm);
    }
    settings->endArray();
}


void freq_markers::Save(QSettings *settings)
{
    int nentries = m_markers.size();
    settings->beginWriteArray("freq_markers",nentries);
    for(int c = 0; c < m_markers.size(); c++ )
    {
        settings->setArrayIndex(c);
        m_markers.at(c)->Save(settings);
    }
    settings->endArray();
}


void freq_markers::OnMarkerChanged(ftmarker *mrk)
{
    emit(MarkerChanged(mrk)); // signal to whoever is listening
}

freq_markers::freq_markers(QObject *parent)
{
    Q_UNUSED(parent);
}

bool freq_markers::Load(char *fname)
{
    QSettings settings(fname);
    Load(&settings);
    return true;
}

bool freq_markers::Save(char *fname)
{
    QSettings settings(fname);
    Save(&settings);
    return true;
}


void freq_markers::AddMarker(ftmarker * mrk)
{
    m_markers.append(mrk);
    connect(mrk,SIGNAL(MarkerChanged(ftmarker*)),this,SLOT(OnMarkerChanged(ftmarker*)));
    //emit(MarkerChanged(mrk));  // both?
    emit(MarkerAdded(mrk));
}

void freq_markers::AddMarkers(QVector<ftmarker *> markers)
{
    m_markers.append(markers);
    for(int c = 0; c < markers.size(); c ++)
    {
        ftmarker * mrk = markers.at(c);
        connect(mrk,SIGNAL(MarkerChanged(ftmarker*)),this,SLOT(OnMarkerChanged(ftmarker*)));
    }
    emit(MarkersAdded(markers));
}


void freq_markers::RemoveMarker(ftmarker *mrk)
{
    int idx = m_markers.indexOf(mrk);
    if(idx == -1)
        return;
    m_markers.remove(idx);
    //m_markers.removeOne(mrk);
    disconnect(mrk,SIGNAL(MarkerChanged(ftmarker*)),this,SLOT(OnMarkerChanged(ftmarker*)));
    emit(MarkerRemoved(mrk));
}

void freq_markers::Clear()
{
    m_markers.clear();
   // emit(MarkerRemoved(0)); // all markers removed
}


bool valueInRange(float value, float min, float max)
{
  //  if(max < min)
  //      printf("damn\r\n");

    return (value >= min) && (value <= max);
}

bool rectOverlap(ftmarker *A, ftmarker *B)
{
    double AX,BX,AY,BY;
    double AW,BW,AH,BH;
    double BX2,BY2,AX2,AY2;
    AX = A->FreqLowMHz();
    AX2 = A->FreqHighMHz();
    if(AX2 < AX)std::swap(AX,AX2);

    AW = AX2 - AX;

    AY = A->StartTime_S();
    AY2 = A->EndTime_S();
    if(AY2 < AY)std::swap(AY,AY2);

    AH = AY2-AY;

    BX = B->FreqLowMHz();
    BX2 = B->FreqHighMHz();
    if(BX2 < BX)std::swap(BX,BX2);
    BW = BX2 - BX;

    BY = B->StartTime_S();
    BY2 = B->EndTime_S();
    if(BY2 < BY)std::swap(BY,BY2);
    BH = BY2 - BY;



    bool xOverlap = valueInRange(AX, BX, BX + BW) ||
                    valueInRange(BX, AX, AX + AW);

    bool yOverlap = valueInRange(AY, BY, BY + BH) ||
                    valueInRange(BY, AY, AY + AH);

    return xOverlap && yOverlap;
}


QVector<ftmarker *> freq_markers::Intersects(ftmarker * mrk)
{
    QVector<ftmarker *> ret;
    for(int c = 0 ; c< m_markers.size(); c++)
    {
        ftmarker *ftm = m_markers[c];
        if(rectOverlap(mrk,ftm))
        {
            ret.append(ftm);
        }
    }
    return ret;
}

