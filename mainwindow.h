#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <sweeper.h>
#include <QTimer>
#include <wgt_fft.h>
#include <wgt_waterfall.h>
#include <sdr_device_manager.h>
#include <ftmarker.h>
//#include <wgt_marker_table.h>
class wgt_marker_table;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    SDR_Device_Manager *m_radios;
    Sweeper *m_sweeper;
    QTimer m_viewtimer; // to update the screen
    wgt_FFT *plotFFT;
    wgt_waterfall *plotWaterfall;
    wgt_marker_table *markerstable;
    freq_markers *m_markers;
    ftmarker m_mrk_main;
private:
    Ui::MainWindow *ui;
    void SetupGUIforRadio();
public slots:
    void onViewTimer();
    void onSweepLineCompleted();
    void onActionRadioMenu();
    void onActionMenuExit();

    void onMarkerSelected(ftmarker *mrk); // hard select - like a goto or double click
    void onMarkerHighlight(ftmarker *mrk); // soft select - like a table item change
    void onAddMarker();
    void onRemoveAllMarkers();
    void onRangeChanged(QCPRange range);

private slots:
    void on_cmdStartStop_clicked();
    void on_chkFFT_clicked();
    void on_chkWaterfall_clicked();
    void on_chkMarkers_clicked();
    void on_chkShowMax_clicked();
    void on_cmdClearMax_clicked();
    void on_sldAverage_valueChanged(int value);
    void on_cmbGain_currentIndexChanged(int index);
    void on_sldGain_valueChanged(int value);
    void on_chkAGC_clicked();
    void on_sldBins_valueChanged(int value);
    void on_spnFreqLow_valueChanged(double arg1);
    void on_spnFreqHigh_valueChanged(double arg1);
};

#endif // MAINWINDOW_H