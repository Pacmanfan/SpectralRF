#ifndef FREQ_TIME_MARKER_H
#define FREQ_TIME_MARKER_H

#include<QSettings>
#include <QObject>
#include <QVector>

/*
This is a frequency and time marker.
It's used to denote time and frequency positions inside an IQ stream
these should be tied to the GUI markers in some way, maybe by raising signals?
*/

class ftmarker:  public QObject
{
    Q_OBJECT
public:
    explicit ftmarker(QObject *parent =0);

    double CF_Hz(); // get the center frequency in Hertz
    void setCF_Hz(double cf);

    double CF_MHz(); // get the center frequency in MegaHertz
    void setCF_MHz(double cf);

    double BW_Hz(); // get bandwidth in Hz
    void setBW_Hz(double BW); // set bandwidth in Hz

    double BW_MHz(); // get bandwidth in MHz
    void setBW_MHz(double BWMHz); // set bandwidth in MHz

    double FreqLowHz() const;
    void setFreqLowHz(double FreqLowHz);

    double FreqLowMHz() const;
    void setFreqLowMHz(double FreqLowMHz);

    double FreqHighHz() const;
    void setFreqHighHz(double FreqHighHz);

    double FreqHighMHz() const;
    void setFreqHighMHz(double FreqHighMHz);

    double StartTime_S() const;
    void setStartTime_S(double StartTime_S);

    double EndTime_S() const;
    void setEndTime_S(double EndTime_S);

    double Duration_S(){return EndTime_S() - StartTime_S();}

    bool HasStartTime(){return m_HasStartTime;}
    void setHasStartTime(bool val)
    {
        m_HasStartTime = val;
        emit(MarkerChanged(this));
    }

    bool HasEndTime(){return m_HasEndTime;}
    void setHasEndTime(bool val)
    {
        m_HasEndTime = val;
        emit(MarkerChanged(this));
    }

    void CopyFrom(ftmarker *ftm);

    //save and load to settings file
    void Load(QSettings *settings);
    void Save(QSettings *settings);
    QString Name() const;
    void setName(const QString &name);

    QString Tags() const;
    void setTags(const QString &Tags);


    void SurpressSignal();
    void RestoreSignal(bool signalnow);

    bool visible() const;

    void setVisible(bool visible);

private:
    double m_FreqLowHz;
    double m_FreqHighHz;
    double m_StartTimeS; // start time in Seconds, -1 for no start time
    double m_EndTimeS; // start time in Seconds, -1 for no end time
    QString m_Name; // marker name
    QString m_Tags; // tags associated with this marker
    bool m_HasStartTime; // does this have a start time?
    bool m_HasEndTime;  // does this have an end time?
    bool m_surpress;
    bool m_visible{true};
    void Signal();

signals:
    void MarkerChanged(ftmarker *mrk); // emitted whenever something in the marker changes

};


/*
This class is a list of markers that can be loaded / saved
it also gives notification when a marker is changed
*/
class freq_markers:  public QObject
{
    Q_OBJECT
public:
    QVector<ftmarker *> m_markers;
    QString m_filename; // the filename that this marker set uses to load / save
    explicit freq_markers(QObject *parent = 0);
    bool Load();
    bool Save();

    void Load(QSettings *settings);
    void Save(QSettings *settings);

    void AddMarker(ftmarker * mrk);
    void AddMarkers(QVector<ftmarker *> markers); //add a group of markers
    void RemoveMarker(ftmarker * mrk);
    void Clear(); // remove all markers
    void Select(ftmarker *mrk); // select this marker
    QVector<ftmarker *> Intersects(ftmarker * mrk); // selector
    QString filename() const;
    void setFilename(const QString &filename);

private slots:
    void OnMarkerChanged(ftmarker *mrk);
signals:
    void MarkerChanged(ftmarker *mrk);
    void MarkerAdded(ftmarker *mrk);
    void MarkersAdded(QVector<ftmarker *> markers); // signal that a group of markers were added
    void MarkerRemoved(ftmarker *mrk);
    void MarkersCleared(); // all markers removed
    void MarkerSelected(ftmarker *mrk);
};

#endif // FREQ_TIME_MARKER_H
