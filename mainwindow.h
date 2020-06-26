#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <sweeper.h>
#include <QTimer>
#include <wgt_fft.h>
#include <wgt_waterfall.h>
#include <sdr_device_manager.h>
#include <ftmarker.h>
#include <signaldetector.h>

#define APP_VERSION "1.0.0.0"
#define APP_NAME "Spectral RF"
#define SETTINGS_FILE "SpectralRF.ini"

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
    SignalDetector *m_signaldetector;
private:
    Ui::MainWindow *ui;
    void SetupGUIforRadio();
    void SetupToolBar();
    void LoadSettings();
    void SaveSettings();
    void StartSweep(bool record);
    void StopSweep();
public slots:
    void onViewTimer();
    void onSweepLineCompleted();
    void onActionRadioMenu();
    void onActionMenuExit();
    void onActionRecord();
    void onActionPlay();
    void onActionStop();

    void onAddMarker();// signal recevied from markers table to add new marker from the fft gui selector
    void onRangeChanged(QCPRange range);
    void OnFreqHighlight(double freq);
    void onLoadMarkers(); // signal recevied from markers table to load new markers file
    void onSaveMarkers(); // signal recevied from markers table to save markers file

private slots:
   // void on_cmdStartStop_clicked();
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
    void on_sldNoiseFloorWidth_valueChanged(int value);
    void on_sldNoiseFloorOffset_valueChanged(int value);
    void on_chkDetectPeaks_clicked();
    void on_sldDetectSensitivity_valueChanged(int value);
    void on_sldOverlap_valueChanged(int value);
    void on_slddbHigh_sliderMoved(int position);
    void on_slddblow_valueChanged(int value);
    void on_cmbWaterfallScheme_currentIndexChanged(int index);

};

#endif // MAINWINDOW_H
