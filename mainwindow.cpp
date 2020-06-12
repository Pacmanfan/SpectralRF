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

bool showTrace = true;
int numbins  = 1048;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Spectrum Profiler");
    setWindowState(Qt::WindowState::WindowMaximized);
    plotFFT = ui->wgtfft;
    plotWaterfall = ui->wgtwaterfall;
    m_sweeper = new Sweeper(this);
    //marker setup
    markerstable = ui->wgtMarkers;
    m_markers = new freq_markers(this);
    markerstable->SetMarkers(m_markers);
    plotFFT->AddTuner(&m_mrk_main);
    plotWaterfall->AddTuner(&m_mrk_main);
    connect(markerstable,SIGNAL(onAddMarker()),this,SLOT(onAddMarker()));
    connect(markerstable,SIGNAL(onRemoveAllMarkers()),this,SLOT(onRemoveAllMarkers()));
    connect(markerstable,SIGNAL(onMarkerHighlight(ftmarker*)),this,SLOT(onMarkerHighlight(ftmarker*)));
    connect(markerstable,SIGNAL(onMarkerSelected(ftmarker*)),this,SLOT(onMarkerSelected(ftmarker*)));

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
    m_viewtimer.start(50);
    //set up a few more connections here.
    connect(plotFFT->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plotWaterfall->plot->xAxis,SLOT(setRange(QCPRange)));
    connect(plotWaterfall->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plotFFT->plot->xAxis,SLOT(setRange(QCPRange)));
    connect(plotFFT->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this,SLOT(onRangeChanged(QCPRange)));

    plotWaterfall->colorMap->setGradient((QCPColorGradient::GradientPreset)9);// 9 == spectrum
    float cf = 850;
    float bw = 100;
    plotFFT->plot->xAxis->setRange(cf - (bw/2),cf + (bw/2)); // bottom
    plotFFT->plot->xAxis2->setRange(cf - (bw/2),cf + (bw/2)); // top
/* //catch out of bounds stuff
    connect(plotFFT->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this,SLOT(FFTrangeChanged(QCPRange)));
    connect(plotWaterfall->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), this,SLOT(FFTrangeChanged(QCPRange)));
*/
    plotWaterfall->setXRange(cf - (bw/2),cf + (bw/2));
    plotWaterfall->setAutorange(true);
    plotWaterfall->setWaterfallDirection(true);

    //start off with the markers table widget not visible
    ui->wgtMarkers->setVisible(false);
    connect(ui->actionRadio,SIGNAL(triggered(bool)),this,SLOT(onActionRadioMenu()));
    connect(ui->actionExit,SIGNAL(triggered(bool)),this,SLOT(onActionMenuExit()));
    on_sldBins_valueChanged(ui->sldBins->value());
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

void MainWindow::onViewTimer()
{

    if(m_sweeper->IsSweeping() == false)
        return;
    //update the waterfall / fft views
    plotWaterfall->Update(m_sweeper->m_ffthist,0,256);
    plotWaterfall->plot->replot();

    plotFFT->UpdateFFT(m_sweeper->m_ffthist);
    plotFFT->plot->replot();

}

void MainWindow::onSweepLineCompleted()
{
    /*
    if(m_sweeper->IsSweeping() == false)
        return;
    //update the waterfall / fft views
    plotWaterfall->Update(m_sweeper->m_ffthist,0,256);
    plotWaterfall->plot->replot();

    plotFFT->UpdateFFT(m_sweeper->m_ffthist);
    plotFFT->plot->replot();
    */
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

void MainWindow::onMarkerSelected(ftmarker *mrk)
{

}

void MainWindow::onMarkerHighlight(ftmarker *mrk)
{

}

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
    plotWaterfall->AddMarker(mrk);
}

void MainWindow::onRemoveAllMarkers()
{
    plotWaterfall->ClearMarkers();
}

void MainWindow::onRangeChanged(QCPRange range)
{
    double low = range.lower;
    double high = range.upper;
    m_sweeper->SetViewRange(low,high);
}

void MainWindow::on_cmdStartStop_clicked()
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
        m_sweeper->StartSweep(ui->spnFreqLow->value()*oneM,ui->spnFreqHigh->value()*oneM,numbins,overlap);
        ui->grpSweepFreqControls->setEnabled(false);

    }else
    {
        m_sweeper->StopSweep();
        ui->grpSweepFreqControls->setEnabled(true);
    }
    if(m_sweeper->IsSweeping()==false)
    {
        ui->cmdStartStop->setText("Start Sweep");
    }else
    {
        ui->cmdStartStop->setText("Stop Sweep");
    }
}

void MainWindow::on_chkFFT_clicked()
{
    ui->wgtfft->setVisible(ui->chkFFT->isChecked());
}

void MainWindow::on_chkWaterfall_clicked()
{
    ui->wgtwaterfall->setVisible(ui->chkWaterfall->isChecked());
}

void MainWindow::on_chkMarkers_clicked()
{
     ui->wgtMarkers->setVisible(ui->chkMarkers->isChecked());
}

void MainWindow::on_chkShowMax_clicked()
{
    showTrace = ui->chkShowMax->isChecked();
    QCPGraph *maxgraph = ui->wgtfft->GetGraph(FFT_MAX_GRAPH_NAME);
    if(maxgraph != nullptr)
    {
        maxgraph->setVisible(showTrace);
    }
}

void MainWindow::on_cmdClearMax_clicked()
{
    m_sweeper->m_ffthist->ClearMaxValues();
}


void MainWindow::on_sldAverage_valueChanged(int value)
{
    double val = value;
    val /= 1000;
    m_sweeper->m_ffthist->SetAlpha(val);
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
}

void MainWindow::on_chkAGC_clicked()
{
    bool val = ui->chkAGC->isChecked();
    SDR_Device * dev = m_sweeper->m_radio;
    if(!dev)
        return;
    dev->sdr->setGainMode(SOAPY_SDR_RX,0,val);
}

void MainWindow::on_sldBins_valueChanged(int value)
{
    numbins = 2<<value;
    SDR_Device * dev = m_sweeper->m_radio;
    if(!dev)
        return;
    double rbw = dev->m_BW_Hz / (double)numbins;

    ui->lblBins->setText("Bins / FFT: " + QString::number(numbins));
    ui->lblRBW->setText("Res. BW: " + QString::number(rbw,'f',2) + " Hz");
    float bw = (ui->spnFreqHigh->value() - ui->spnFreqLow->value())/2.0;;
    double binsperscan = (bw*1000000) / rbw;
    ui->lblBinperscan->setText("Bins per scan: " + QString::number((int)binsperscan));

}

void MainWindow::on_spnFreqLow_valueChanged(double arg1)
{
    on_sldBins_valueChanged(ui->sldBins->value());
}

void MainWindow::on_spnFreqHigh_valueChanged(double arg1)
{
    on_sldBins_valueChanged(ui->sldBins->value());
}

