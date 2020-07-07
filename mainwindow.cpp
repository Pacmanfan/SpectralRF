#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>

#include <string>	// std::string
#include <vector>	// std::vector<...>
#include <map>		// std::map< ... , ... >
#include <iostream>
#include <radiosettings.h>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>

bool showTrace = true;
#define DEF_NUM_BINS 4096
int gNumBins = DEF_NUM_BINS;
#define VIEWTIMERRES 50
bool detectingpeaks = false;
QString gSettingsFileName;
//QString gMarkersFileName;
bool loadingsettings = false;


#define DEFAULT_MARKERS_FILENAME "markers.mrk"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    loadingsettings = true; // prevent saving of any settings until the initialization is complete
    ui->setupUi(this);
    gSettingsFileName = QApplication::applicationDirPath() + "/" + SETTINGS_FILE;
    //gMarkersFileName =  QApplication::applicationDirPath() + "/" + DEFAULT_MARKERS_FILENAME;
    setWindowTitle(QString(APP_NAME) + " " + QString(APP_VERSION));
    setWindowState(Qt::WindowState::WindowMaximized);
    plotFFT = ui->wgtfft;
    plotWaterfall = ui->wgtwaterfall;
    m_sweeper = new Sweeper(this);
    //marker setup
    markerstable = ui->wgtMarkers;
    m_markers = new freq_markers(this);// create new markers

    markerstable->SetMarkers(m_markers); // set the pointer to the markers on the markers table
    plotWaterfall->setMarkers(m_markers); // set the markers to the waterfall
    plotFFT->setMarkers(m_markers);
    m_markers->setFilename(QApplication::applicationDirPath() + "/" + DEFAULT_MARKERS_FILENAME);


    plotFFT->AddMainTuner(&m_mrk_main); // so it doesn't get wiped
    plotWaterfall->AddTuner(&m_mrk_main);

    connect(markerstable,SIGNAL(onAddMarker()),this,SLOT(onAddMarker()));
    connect(markerstable,SIGNAL(onLoadMarkers()),this,SLOT(onLoadMarkers()));
    connect(markerstable,SIGNAL(onSaveMarkers()),this,SLOT(onSaveMarkers()));

    //create  new radio device manager
    m_radios = new SDR_Device_Manager();
    //detect all the radios in the system
    m_radios->DetectAll();
    //check for at least 1 radio
    if(m_radios->m_devices.size() > 0)
    {
        m_sweeper->SetRadio(m_radios->m_devices[0]); // first radio
    }else
    {
        QMessageBox::information(this,"No Radio Detected","Check your radio connection.");
    }
    //m_sweeper->InitSDR();

    SetupGUIforRadio();
    connect(m_sweeper,SIGNAL(SweepCompleted()),this,SLOT(onSweepLineCompleted()));
    connect(&m_viewtimer,SIGNAL(timeout()),this,SLOT(onViewTimer()));
    //set up a few more connections here.
    connect(plotFFT->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plotWaterfall->plot->xAxis,SLOT(setRange(QCPRange)));
    connect(plotWaterfall->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plotFFT->plot->xAxis,SLOT(setRange(QCPRange)));
    connect(plotFFT->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this,SLOT(onRangeChanged(QCPRange)));
    connect(plotFFT,SIGNAL(OnFreqHighlight(double)),this,SLOT(OnFreqHighlight(double)));
    connect(plotWaterfall,SIGNAL(OnFreqHighlight(double)),this,SLOT(OnFreqHighlight(double)));

    plotWaterfall->colorMap->setGradient((QCPColorGradient::GradientPreset)9);// 9 == spectrum
    float cf = 850;
    float bw = 100;
    plotFFT->plot->xAxis->setRange(cf - (bw/2),cf + (bw/2)); // bottom
    plotFFT->plot->xAxis2->setRange(cf - (bw/2),cf + (bw/2)); // top
    plotFFT->setAutorange(false);

/* //catch out of bounds stuff
    connect(plotFFT->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this,SLOT(FFTrangeChanged(QCPRange)));
    connect(plotWaterfall->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this,SLOT(FFTrangeChanged(QCPRange)));
*/
    plotWaterfall->setXRange(cf - (bw/2),cf + (bw/2));
    plotWaterfall->setAutorange(false);
    plotWaterfall->setWaterfallDirection(true);

    //start off with the markers table widget not visible
    ui->wgtMarkers->setVisible(false);
    connect(ui->actionRadio,SIGNAL(triggered(bool)),this,SLOT(onActionRadioMenu()));
    connect(ui->actionExit,SIGNAL(triggered(bool)),this,SLOT(onActionMenuExit()));
    connect(ui->actionRecord,SIGNAL(triggered(bool)),this,SLOT(onActionRecord()));
    connect(ui->actionPlay,SIGNAL(triggered(bool)),this,SLOT(onActionPlay()));
    connect(ui->actionStop,SIGNAL(triggered(bool)),this,SLOT(onActionStop()));
    //on_sldBins_valueChanged(DEF_NUM_BINS);
    SetupToolBar();

    float lower = ui->slddblow->value();
    float upper = ui->slddbHigh->value();
    plotFFT->setRangeY(lower,upper);
    plotWaterfall->setRange_dBm(lower,upper);

    LoadSettings(); // markers loaded here
    ui->actionRecord->setEnabled(true);
    ui->actionPlay->setEnabled(true);
    ui->actionStop->setEnabled(false);
    m_signaldetector = new SignalDetector();
    m_signaldetector->SetFFTSource(m_sweeper->m_ffthist);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetupGUIforRadio()
{
    SDR_Device * dev = m_sweeper->m_radio;
    if(!dev)
        return;

    //set the min / max ranges
    double oneM = 1000000;
    ui->spnFreqLow->setMinimum(dev->m_freq_limit_low/oneM);
    ui->spnFreqLow->setMaximum(dev->m_freq_limit_high/oneM);
    ui->spnFreqHigh->setMinimum(dev->m_freq_limit_low/oneM);
    ui->spnFreqHigh->setMaximum(dev->m_freq_limit_high/oneM);
    //set the current values to be min / max
    ui->spnFreqLow->setValue(800);
    ui->spnFreqHigh->setValue(900);
    ui->cmbAntenna->clear();
    for(unsigned int c = 0; c < dev->lst_ant.size(); c++)
    {
        ui->cmbAntenna->addItem(dev->lst_ant[c].c_str());
    }
    ui->cmbGain->clear();
    for(unsigned int c = 0; c < dev->lst_gain.size() ; c ++)
    {
        ui->cmbGain->addItem(dev->lst_gain[c].c_str());
    }
    bool autogain = dev->sdr->getGainMode(SOAPY_SDR_RX,0);
    ui->chkAGC->setChecked(autogain);

}
int gtest = 0;
void MainWindow::LoadSettings()
{
    loadingsettings = true;
    QSettings settings(gSettingsFileName, QSettings::NativeFormat);

    ui->spnFreqLow->setValue(settings.value("FreqSweepLow", 800).toDouble());
    ui->spnFreqHigh->setValue(settings.value("FreqSweepHigh", 900).toDouble());
    bool bval = settings.value("DetectPeaks",false).toBool();
    ui->chkDetectPeaks->setChecked(bval);
    ui->sldNoiseFloorOffset->setValue(settings.value("sldNoiseFloorOffset",ui->sldNoiseFloorOffset->value()).toInt());
    ui->sldNoiseFloorWidth->setValue(settings.value("sldNoiseFloorWidth",ui->sldNoiseFloorWidth->value()).toInt());
    ui->sldDetectSensitivity->setValue(settings.value("sldDetectSensitivity",ui->sldDetectSensitivity->value()).toInt());
    ui->sldOverlap->setValue(settings.value("sldOverlap",ui->sldOverlap->value()).toInt());
    ui->sldGain->setValue(settings.value("sldGain",ui->sldGain->value()).toInt());
    ui->slddblow->setValue(settings.value("slddblow",ui->slddblow->value()).toInt());
    ui->slddbHigh->setValue(settings.value("slddbHigh",ui->slddbHigh->value()).toInt());
    ui->cmbWaterfallScheme->setCurrentIndex(settings.value("cmbWaterfallScheme",ui->cmbWaterfallScheme->currentIndex()).toInt());
    m_markers->setFilename(settings.value("MarkerFilename",QApplication::applicationDirPath() + "/" + DEFAULT_MARKERS_FILENAME).toString());
    m_markers->Load();
    int sldbin = settings.value("NumBins",ui->sldBins->value()).toInt();
    on_sldBins_valueChanged(sldbin);

    loadingsettings = false;
}

void MainWindow::SaveSettings()
{
    if(loadingsettings == true)
        return;
    QSettings settings(gSettingsFileName, QSettings::NativeFormat);

    settings.setValue("FreqSweepLow", ui->spnFreqLow->value());
    settings.setValue("FreqSweepHigh", ui->spnFreqHigh->value());
    settings.setValue("NumBins",ui->sldBins->value());
    settings.setValue("DetectPeaks",ui->chkDetectPeaks->isChecked());
    settings.setValue("sldNoiseFloorOffset",ui->sldNoiseFloorOffset->value());
    settings.setValue("sldNoiseFloorWidth",ui->sldNoiseFloorWidth->value());
    settings.setValue("sldDetectSensitivity",ui->sldDetectSensitivity->value());
    settings.setValue("sldOverlap",ui->sldOverlap->value());
    settings.setValue("slddblow",ui->slddblow->value());
    settings.setValue("slddbHigh",ui->slddbHigh->value());
    settings.setValue("cmbWaterfallScheme",ui->cmbWaterfallScheme->currentIndex());
    settings.setValue("MarkerFilename",m_markers->m_filename);

}

void MainWindow::StartSweep(bool record)
{
    if(m_sweeper->IsSweeping()==false)
    {
        if(ui->spnFreqLow->value() > ui->spnFreqHigh->value())
        {
            QMessageBox::information(this,"Frequency mismatch","Low frequency is greater than high frequency.");
            return;
        }
        //validate to see if low > high (todo)
        double oneM = 1000000;
        float bw = (ui->spnFreqHigh->value() - ui->spnFreqLow->value())/2.0;
        float cf = (ui->spnFreqHigh->value() + ui->spnFreqLow->value())/2.0;

        m_mrk_main.setCF_MHz(cf);
        m_mrk_main.setBW_MHz(1);

        plotFFT->plot->xAxis->setRange(cf - (bw/2),cf + (bw/2));
        plotFFT->plot->xAxis2->setRange(cf - (bw/2),cf + (bw/2));
        plotFFT->plot->update();
        plotWaterfall->setXRange(cf - (bw/2),cf + (bw/2));
        plotWaterfall->setRange_dBm(-140,-30);
        plotWaterfall->plot->update();

        int overlap = ui->sldOverlap->value();
        m_sweeper->StartSweep(ui->spnFreqLow->value()*oneM,ui->spnFreqHigh->value()*oneM,gNumBins,overlap);

    }
    if(m_sweeper->IsSweeping()==true)
    {
        m_viewtimer.start(VIEWTIMERRES);

        ui->grpSweepFreqControls->setEnabled(false);
        ui->actionRecord->setEnabled(false);
        ui->actionPlay->setEnabled(false);
        ui->actionStop->setEnabled(true);
    }
}

void MainWindow::StopSweep()
{
    m_viewtimer.stop();
    //probably need a wait here to make sure we're not still drawing...
    int c= 0;

    if(m_sweeper->IsSweeping()==true)
    {
        m_sweeper->StopSweep();
    }
  //  if(m_sweeper->IsSweeping()==false)
    {
        ui->grpSweepFreqControls->setEnabled(true);
        ui->actionRecord->setEnabled(true);
        ui->actionPlay->setEnabled(true);
        ui->actionStop->setEnabled(false);
    }
}

void MainWindow::SetupToolBar()
{

}

void MainWindow::onViewTimer()
{
    if(m_sweeper->IsSweeping() == false)
        return;

    //QMap<int,float> peaks;
    if(detectingpeaks)
    {

        float v = ui->sldDetectSensitivity->value();
        v /= 100.0;
        /*
       // v=3.25;
        peaks = m_sweeper->m_ffthist->DetectPeaks(v);
        if(peaks.count() > 0)
        {
            //printf("%d peaks\r\n",peaks.count());
        }
        //now, display the peaks on the fft graph
        */
        m_signaldetector->Update(v);
    }
    //update the waterfall / fft views

    plotWaterfall->Update(m_sweeper->m_ffthist,0,256);
    plotWaterfall->plot->replot();

    plotFFT->UpdateFFT(m_sweeper->m_ffthist);
    plotFFT->plot->replot();

}

void MainWindow::onSweepLineCompleted()
{

}

void MainWindow::onActionRadioMenu()
{
    RadioSettings settings(this);
    settings.Setup(m_radios);
    settings.exec();
}

void MainWindow::onActionMenuExit()
{
    QApplication::quit();
}

void MainWindow::onActionRecord()
{
    //start the sweep and start recording
    //StartSweep(true);
}

void MainWindow::onActionPlay()
{
    StartSweep(false);
}

void MainWindow::onActionStop()
{
    StopSweep();
}



/*
This is triggered when the markers table adds a new marker
*/
void MainWindow::onAddMarker()
{
    //create a new marker, add it to the markers table
    //make sure it's added to the waterfall
    ftmarker *mrk = new ftmarker(this);
    mrk->setCF_MHz(m_mrk_main.CF_MHz());
    mrk->setBW_MHz(m_mrk_main.BW_MHz());
    mrk->setHasEndTime(false);
    mrk->setHasStartTime(false);
    m_markers->AddMarker(mrk);
}


void MainWindow::onRangeChanged(QCPRange range)
{
    double low = range.lower;
    double high = range.upper;
    m_sweeper->SetViewRange(low,high);
}

void MainWindow::OnFreqHighlight(double freqMHz)
{
    //format the string
    QString txt;
    txt = QString::number(freqMHz,'f',3) + " MHz";
    ui->actionFrequency->setText(txt);
}

void MainWindow::onLoadMarkers()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open Markers File", QApplication::applicationDirPath(), "Marker Files (*.mrk)");
    if(fileName.length() > 0)
    {
        m_markers->setFilename(fileName);
        //load the markers file
        m_markers->Clear();
        m_markers->Load();
    }
}

void MainWindow::onSaveMarkers()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Markers File", QApplication::applicationDirPath(), "Marker Files (*.mrk)");
    if(fileName.length() > 0)
    {
        m_markers->setFilename(fileName);        
        m_markers->Save();//save the markers file
    }
}


void MainWindow::on_chkFFT_clicked()
{
    ui->wgtfft->setVisible(ui->chkFFT->isChecked());
    SaveSettings();
}

void MainWindow::on_chkWaterfall_clicked()
{
    ui->wgtwaterfall->setVisible(ui->chkWaterfall->isChecked());
    SaveSettings();
}

void MainWindow::on_chkMarkers_clicked()
{
     ui->wgtMarkers->setVisible(ui->chkMarkers->isChecked());
     SaveSettings();
}

void MainWindow::on_chkShowMax_clicked()
{
    showTrace = ui->chkShowMax->isChecked();
    QCPGraph *maxgraph = ui->wgtfft->GetGraph(FFT_MAX_GRAPH_NAME);
    if(maxgraph != nullptr)
    {
        maxgraph->setVisible(showTrace);
    }
    SaveSettings();
}

void MainWindow::on_cmdClearMax_clicked()
{
    m_sweeper->m_ffthist->ClearMinMaxValues();
}


void MainWindow::on_sldAverage_valueChanged(int value)
{
    double val = value;
    val /= 1000;
    m_sweeper->m_ffthist->SetAlpha(val);
    SaveSettings();
}

void MainWindow::on_cmbGain_currentIndexChanged(int index)
{
    SDR_Device * dev = m_sweeper->m_radio;
    if(!dev)
        return;
    double gain = dev->sdr->getGain(SOAPY_SDR_RX,index);
    SoapySDR::Range grange = dev->sdr->getGainRange(SOAPY_SDR_RX,index);
    ui->sldGain->setMinimum(grange.minimum());
    ui->sldGain->setMaximum(grange.maximum());
    ui->sldGain->setValue(gain);
    SaveSettings();
}

void MainWindow::on_sldGain_valueChanged(int value)
{
    SDR_Device * dev = m_sweeper->m_radio;
    if(!dev)
        return;
    int idx = ui->cmbGain->currentIndex();
    if(idx == -1)
        return;
    dev->sdr->setGain(SOAPY_SDR_RX,idx,value);
    SaveSettings();
}

void MainWindow::on_chkAGC_clicked()
{
    bool val = ui->chkAGC->isChecked();
    SDR_Device * dev = m_sweeper->m_radio;
    if(!dev)
        return;
    dev->sdr->setGainMode(SOAPY_SDR_RX,0,val);
    SaveSettings();
}

void MainWindow::on_sldBins_valueChanged(int value)
{
    gNumBins = 2<<value;
    SDR_Device * dev = m_sweeper->m_radio;
    if(!dev)
        return;
    double rbw = dev->m_BW_Hz / (double)gNumBins;

    ui->lblBins->setText("Bins / FFT: " + QString::number(gNumBins));
    ui->lblRBW->setText("Res. BW: " + QString::number(rbw,'f',2) + " Hz");
    float bw = (ui->spnFreqHigh->value() - ui->spnFreqLow->value())/2.0;;
    double binsperscan = (bw*1000000) / rbw;
    ui->lblBinperscan->setText("Bins per scan: " + QString::number((int)binsperscan));
    SaveSettings();
}

void MainWindow::on_spnFreqLow_valueChanged(double arg1)
{
    on_sldBins_valueChanged(ui->sldBins->value());
    SaveSettings();
}

void MainWindow::on_spnFreqHigh_valueChanged(double arg1)
{
    on_sldBins_valueChanged(ui->sldBins->value());
    SaveSettings();
}


void MainWindow::on_sldNoiseFloorWidth_valueChanged(int value)
{
    m_sweeper->m_ffthist->setNoise_floor_binwidth(value);
    SaveSettings();
}

void MainWindow::on_sldNoiseFloorOffset_valueChanged(int value)
{
    double v = value - 50;
    v /= 10.0;
    m_sweeper->m_ffthist->setNoise_floor_offset(v);
    SaveSettings();
}

void MainWindow::on_chkDetectPeaks_clicked()
{
    detectingpeaks = ui->chkDetectPeaks->isChecked();
    SaveSettings();
}

void MainWindow::on_sldDetectSensitivity_valueChanged(int value)
{
    SaveSettings();
}

void MainWindow::on_sldOverlap_valueChanged(int value)
{
     SaveSettings();
}

void MainWindow::on_slddbHigh_sliderMoved(int position)
{
    //set the dbm high position for the fft and the waterfall
    float lower = ui->slddblow->value();
    float upper = ui->slddbHigh->value();
    plotFFT->setRangeY(lower,upper);
    plotWaterfall->setRange_dBm(lower,upper);
    SaveSettings();
}

void MainWindow::on_slddblow_valueChanged(int value)
{
    //set the dbm high position for the fft and the waterfall
    float lower = ui->slddblow->value();
    float upper = ui->slddbHigh->value();
    plotFFT->setRangeY(lower,upper);
    plotWaterfall->setRange_dBm(lower,upper);
    SaveSettings();
}


void MainWindow::on_cmbWaterfallScheme_currentIndexChanged(int index)
{
    plotWaterfall->colorMap->setGradient((QCPColorGradient::GradientPreset)index);
    SaveSettings();
}
