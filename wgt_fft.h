#ifndef WGT_FFT_H
#define WGT_FFT_H

#include <QWidget>
#include <wgt_sig_base.h>
#include <ftmarker.h>
#include <fft_hist.h>
#include <sigtuner_gui.h>

#define FFT_GRAPH_NAME "FFT"
#define FFT_MAX_GRAPH_NAME "MaxFFT"
#define FFT_AVG_GRAPH_NAME "AvgFFT"

namespace Ui {
class wgt_FFT;
}

// Now, we want some sort of on-control editable ways to choose the CF and BW
// previously, we showed a single selector, this was used to change the window filter
// in the future, this could be the position of multiple tuners
// we should probably allow sigtuners to be added to the control, and the control will create
// and manage the appropriate

class wgt_FFT : public QWidget, public wgt_Sig_Base
{
    Q_OBJECT

public:
    explicit wgt_FFT(QWidget *parent = 0);
    ~wgt_FFT();
    void Initialize();
    mouseinfo miFFT; // current and last mouse info
    bool autorange; // automatically scale to fit the data on screen in the Y axis
    double rangelowY,rangehighY; // when NOT autoranging, use the set values


    QCPItemTracer *_FFTTracer; // the tracer on the FFT graph
    void UpdateCursorDotPosition(double xCenterIn);
    void SetXRange(float lower,float upper);
    void SetControlsVisible(bool val);
    void UpdateFFT(FFT_Hist *pFFTHelp);
    bool getAutorange() const;
    void setAutorange(bool value);

    void setRangeY(double low, double high);

    void AddTuner(ftmarker *tunermarker);
    void RemoveTuner(ftmarker *ftm);

private:
    Ui::wgt_FFT *ui;
    QVector<sigtuner_GUI *> m_tuners_gui;
    double lowfreq,highfreq;
public slots:
    void OnMouseDoubleClickFFT(QMouseEvent *evt);
    void OnMousePressedFFT(QMouseEvent *evt);
    void OnMouseMoveFFT(QMouseEvent *evt);
    void OnMouseReleaseFFT(QMouseEvent *evt);
signals:
    void OnFreqHighlight(double freq);
};

#endif // WGT_FFT_H
