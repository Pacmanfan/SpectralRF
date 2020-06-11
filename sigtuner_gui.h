#ifndef SIGTUNER_GUI_H
#define SIGTUNER_GUI_H

#include <qcustomplot.h>
#include <ftmarker.h>

/*

This is the GUI representation of a tuner on either the FFT plot
or the waterfall plot.

Used in conjection with the control, it will allow editing CF/BW changes

*/


// enum for line selection
enum linesel
{
    eMV_LowF,
    eMV_HighF,
    eMV_CenterF,
    eMV_None
};

// this is a qobject to receive signals
class sigtuner_GUI :  public QObject
{
    Q_OBJECT
public:
    sigtuner_GUI(ftmarker *tunermarker,QCustomPlot *ParentPlot);
    QCPItemStraightLine *lowFreqLineFFT;
    QCPItemStraightLine *highFreqLineFFT;
    QCPItemStraightLine *CenterFreqLineFFT;

    linesel m_ms; // moving line selection
    QCustomPlot *m_ParentPlot;
    ftmarker *m_tunermarker;
    void SetupLine(QCPItemStraightLine* line, float xv, QColor color);
private slots:
    void OnMarkerChanged(ftmarker *mrk);
};

#endif // SIGTUNER_GUI_H
