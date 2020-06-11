#ifndef WGT_WATERFALL_H
#define WGT_WATERFALL_H

#include <QWidget>
#include <qcustomplot.h>
#include <wgt_sig_base.h>
#include <fft_hist.h>
#include "ftmarker.h"
#include <sigtuner_gui.h>
#include <ftmarker_gui.h>

namespace Ui {
class wgt_waterfall;
}

class wgt_waterfall : public QWidget , public wgt_Sig_Base
{
    Q_OBJECT

public:
    explicit wgt_waterfall(QWidget *parent = 0);
    ~wgt_waterfall();
    QCPColorMap *colorMap; // waterfall plot
    //QCustomPlot *plotWaterfall; // the lower waterfall plot
    mouseinfo miWaterfall; // current and last mouse info
    void Update(FFT_Hist *pFFTHelp, double tlow, double thigh);
    void Initialize();
    bool WaterfallDirection;
    bool getWaterfallDirection() const;
    void setWaterfallDirection(bool value);
    bool autorange; // automatically scale to fit the data on screen in the Y axis
    bool ctrlpressed;
    double waterhigh,waterlow; // when NOT autoranging, use the set values
    void AddMarker(ftmarker *ftm);
    void RemoveMarker(ftmarker *ftm);
    void ClearMarkers();// remove all GUI markers
    void SetControlsVisible(bool val);
    bool getAutorange() const;
    void setAutorange(bool value);
    void setRange_dBm(double low, double high){waterlow = low; waterhigh = high;}
    void setXRange(double low, double high);
    void AddTuner(ftmarker *tunermarker);
    void RemoveTuner(ftmarker *ftm);
    void UnselectMarkers();
    void SetMarkerSelected(ftmarker *ftm);
    void SetMarkersSelected(QVector<ftmarker *> markers);
    ftmarker *GetCurSel(){return m_rectselect;}
    bool HasSelection(){return m_selector->IsVisible();}
private:
    Ui::wgt_waterfall *ui;
    bool eventFilter(QObject *obj, QEvent *event);
    QVector<sigtuner_GUI *> m_tuners_gui;
    QVector<ftmarker_GUI *> m_markers_gui;
    freq_markers m_markers; // copy of the freq markers so we can select from them
    ftmarker_GUI *m_selector; // the rectangular selector
    ftmarker *m_rectselect;
    double lowfreq,highfreq;

private slots:
    void OnMouseMove(QMouseEvent *evt);
    void OnMouseRelease(QMouseEvent *evt);
    void OnMousePressed(QMouseEvent *evt);
    void OnMouseDoubleClick(QMouseEvent *evt);
    void onitemDoubleClick(QCPAbstractItem *item ,QMouseEvent *ev); // single marker selection on double click
    void onitemClick(QCPAbstractItem *item ,QMouseEvent *ev); // single marker selection on double click
signals:
    void OnFreqHighlight(double freq);
    void OnTimeHighlight(double timestamp);
    void Pan(double val);
    void OnMarkersSelected(QVector<ftmarker *> markers);
    void OnMarkerSelected(ftmarker *marker,bool softsel);
    void OnDeleteCurrentMarker();

};

#endif // WGT_WATERFALL_H
