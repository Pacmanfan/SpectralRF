#include "wgt_fft.h"
#include "ui_wgt_fft.h"

wgt_FFT::wgt_FFT(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::wgt_FFT)
{
    ui->setupUi(this);
    Initialize();
    SetControlsVisible(false);
}

wgt_FFT::~wgt_FFT()
{
    delete ui;
}

void wgt_FFT::Initialize()
{

    autorange = true;
    plot = ui->plotFFT;//

    plot->axisRect()->setAutoMargins(QCP::MarginSides());
    plot->axisRect()->setMargins(QMargins(70,20,12,0));

    plot->xAxis2->setVisible(true);  //DISPLAY THE X AXIS ON SPECAN?
    plot->xAxis2->setTickLabels(true);

    plot->xAxis->setVisible(false);  //DISPLAY THE X AXIS ON SPECAN?
    plot->xAxis->setTickLabels(true);

    //Y AXIS (ON LEFT)
    plot->yAxis->setVisible(true);
    plot->yAxis->setLabel("dBm");
    plot->yAxis->setNumberPrecision(3);

    //Y AXIS (ON RIGHT)
    plot->yAxis2->setVisible(false);

    //ADD GRAPH:
    QCPGraph *graph = AddGraph(FFT_GRAPH_NAME,plot->xAxis2,Qt::yellow);  // instantaneous  power
    AddGraph(FFT_MAX_GRAPH_NAME,Qt::red);// max power
    QCPGraph *avgraph  = AddGraph(FFT_AVG_GRAPH_NAME,Qt::green);// average power



    // make left and bottom axes always transfer their ranges to right and top axes:
    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));

    //CONNECT SLOTS TO SIGNALS:

    // mouse move
    connect(plot, SIGNAL(mouseMove(QMouseEvent*)), this,
        SLOT(OnMouseMoveFFT(QMouseEvent*)));

    //SINGLE MOUSE CLICK (used for INPUTING threshold)
    connect(plot, SIGNAL(mousePress(QMouseEvent*)), this,
            SLOT(OnMousePressedFFT(QMouseEvent*)));

    //Mouse Release
    connect(plot, SIGNAL(mouseRelease(QMouseEvent*)), this,
            SLOT(OnMouseReleaseFFT(QMouseEvent*)));

    plot->axisRect()->setBackground(Qt::black);

    //DOUBLE CLICK
    connect(plot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this,
            SLOT(OnMouseDoubleClickFFT(QMouseEvent*)));

        //make things selectable:
    plot->setInteractions( QCP::iRangeZoom | QCP::iRangeDrag);
    plot->axisRect()->setRangeZoomFactor(1.25,1);
    plot->axisRect()->setRangeDrag(Qt::Horizontal);


    QColor colphase = QColor::fromRgb(255,0,0);
    _FFTTracer = new QCPItemTracer(plot);
    _FFTTracer->setGraph(avgraph);
    _FFTTracer->setVisible(true);
    _FFTTracer->setStyle(QCPItemTracer::tsSquare);
    _FFTTracer->setPen(QPen(colphase));   //DOT color
    _FFTTracer->setBrush(colphase);
    _FFTTracer->setSize(7);


}

bool wgt_FFT::getAutorange() const
{
    return autorange;
}

void wgt_FFT::setAutorange(bool value)
{
    autorange = value;
}



void wgt_FFT::setRangeY(double low, double high)
{
    rangelowY = low;
    rangehighY = high;
}

void wgt_FFT::AddTuner(ftmarker *tunermarker)
{
    sigtuner_GUI *stg = new sigtuner_GUI(tunermarker,plot);
    m_tuners_gui.append(stg); // add it to the list for safekeeping

}

void wgt_FFT::RemoveTuner(ftmarker *tunermarker)
{
    // iterate through all and remove
    int idx = -1;
    for(int c= 0; c < m_tuners_gui.size(); c++)
    {
        if(tunermarker == m_tuners_gui[c]->m_tunermarker)
        {
            idx = c;
            break;
        }
    }
    if(idx != -1)
        m_tuners_gui.remove(idx);
}


void wgt_FFT::OnMouseReleaseFFT(QMouseEvent *evt)
{
    Q_UNUSED(evt);

    plot->setInteractions( QCP::iRangeZoom | QCP::iRangeDrag | QCP::iSelectItems);
    for(int c= 0; c < m_tuners_gui.size(); c++ )
    {
        sigtuner_GUI *stg = m_tuners_gui[c];
        stg->m_ms = eMV_None;
        stg->CenterFreqLineFFT->setSelected(false);
        stg->lowFreqLineFFT->setSelected(false);
        stg->highFreqLineFFT->setSelected(false);
    }

}

void wgt_FFT::OnMouseMoveFFT(QMouseEvent *evt)
{
    Q_UNUSED(evt);

    miFFT.Set(evt->pos().x(),evt->pos().y());
    // update the phase tracer position
    float x = plot->xAxis->pixelToCoord(evt->pos().x());
    UpdateCursorDotPosition(x);
    //ui->lblMOFreq->setText("Frequency : " + QString::number(x) + " MHz");
    emit(OnFreqHighlight(x));
    //check and see if we've selected the threshold


    float xp = evt->pos().x();
    float xv = plot->xAxis->pixelToCoord(xp);

    for(int c= 0; c < m_tuners_gui.size(); c++ )
    {
        sigtuner_GUI *stg = m_tuners_gui[c];

        float bw = stg->m_tunermarker->BW_MHz();
        float nbw = 0;
        float cf = stg->m_tunermarker->CF_MHz();
        switch(stg->m_ms)
        {
            case eMV_None:

                break;
            case eMV_CenterF:

                if(xv + (bw / 2) > highfreq)
                {
                    xv = highfreq - (bw / 2);
                }
                if(xv - (bw / 2) < lowfreq)
                {
                    xv = lowfreq + (bw / 2);
                }

                stg->m_tunermarker->setCF_MHz(xv);
                //SetupSelection(stg->m_tunermarker->Sel_centerf,bw);
                break;
            case eMV_HighF:
                stg->m_tunermarker->setFreqHighMHz(xv);
                nbw = stg->m_tunermarker->FreqHighMHz() - stg->m_tunermarker->CF_MHz();
                nbw *= 2;
                nbw = fabs(nbw);

                if(cf + (nbw / 2) >  highfreq)
                {
                    nbw = (highfreq - cf) * 2;
                }
                if(cf - (nbw / 2) <  lowfreq)
                {
                    nbw = (cf - lowfreq) * 2;
                }

                stg->m_tunermarker->setFreqLowMHz(cf - (nbw / 2));
                stg->m_tunermarker->setFreqHighMHz(cf + (nbw / 2));

                //SetupSelection(stg->m_tunermarker->Sel_centerf,nbw);
                break;
            case eMV_LowF:
                stg->m_tunermarker->setFreqLowMHz(xv);
                nbw =  stg->m_tunermarker->CF_MHz() - stg->m_tunermarker->FreqLowMHz();
                nbw *= 2;
                nbw = fabs(nbw);

                if(cf + (nbw / 2) >  highfreq)
                {
                    nbw = (highfreq - cf) * 2;
                }
                if(cf - (nbw / 2) <  lowfreq)
                {
                    nbw = (cf - lowfreq) * 2;
                }

                stg->m_tunermarker->setFreqLowMHz(cf - (nbw / 2));
                stg->m_tunermarker->setFreqHighMHz(cf + (nbw / 2));
                //SetupSelection(stg->m_tunermarker->Sel_centerf,nbw);
                break;
        }

    }

}


void wgt_FFT::OnMouseDoubleClickFFT(QMouseEvent *evt)
{
     Q_UNUSED(evt);
//    int val = plot->xAxis->pixelToCoord(evt->pos().x()) ;
//    float xv = plot->xAxis->pixelToCoord(evt->pos().x());
}



void wgt_FFT::OnMousePressedFFT(QMouseEvent *evt)
{
    miFFT.Set(evt->pos().x(),evt->pos().y());

    float xv = plot->xAxis->pixelToCoord(evt->pos().x());
  //  printf("OnMousePressedFFT on %f\r\n",xv);

    if(evt->button() == Qt::LeftButton)
    {
        for(int c= 0; c < m_tuners_gui.size(); c++ )
        {
            sigtuner_GUI *stg = m_tuners_gui[c];
            int xp;
             // check to see if it's selected
            xp = stg->CenterFreqLineFFT->point1->pixelPosition().x();
            if(evt->pos().x()  > (xp - LINEGRIPWIDTH) && evt->pos().x() < (xp + LINEGRIPWIDTH))
            {
                stg->m_ms = eMV_CenterF;
                stg->CenterFreqLineFFT->setSelected(true);
                plot->setInteraction ( QCP::iRangeDrag, false ); // turn off range drag for now
            }
             // check to see if it's selected
             xp = stg->lowFreqLineFFT->point1->pixelPosition().x();
            if(evt->pos().x()  > (xp - LINEGRIPWIDTH) && evt->pos().x() < (xp + LINEGRIPWIDTH))
            {
                stg->m_ms = eMV_LowF;
                stg->lowFreqLineFFT->setSelected(true);
                plot->setInteraction ( QCP::iRangeDrag, false ); // turn off range drag for now
            }
             // check to see if it's selected
             xp = stg->highFreqLineFFT->point1->pixelPosition().x();
            if(evt->pos().x()  > (xp - LINEGRIPWIDTH) && evt->pos().x() < (xp + LINEGRIPWIDTH))
            {
                stg->m_ms = eMV_HighF;
                stg->highFreqLineFFT->setSelected(true);
                plot->setInteraction ( QCP::iRangeDrag, false ); // turn off range drag for now
            }
        }
    }

}


//PLOT THE RED DOT ON THE SCREEN (TOP PLOT AREA)
void wgt_FFT::UpdateCursorDotPosition(double xCenterIn)
{
    if(_FFTTracer->graph()->data()->size() < 1 )
        return;
    _FFTTracer->setGraphKey(xCenterIn);
}

void wgt_FFT::SetXRange(float lower, float upper)
{
    plot->xAxis->setRange(lower,upper);
    plot->xAxis2->setRange(lower,upper);
}

void wgt_FFT::SetControlsVisible(bool val)
{
    ui->wgt_Controls->setVisible(val);
}

/*
Display the contents of the FFT_Hist in the GUI
*/
void wgt_FFT::UpdateFFT(FFT_Hist *pFFTHelp)
{
    // add data to the vector
    static QVector<float> freqs;
    static QVector<float> datanoisefloor;
    static QVector<float> dataMax;
    static QVector<float> dataAvg;

    if(pFFTHelp !=0)
    {
        int sz = pFFTHelp->GetBinSize();
        if(freqs.size() != sz)
        {
            freqs.resize(sz);
            datanoisefloor.resize(sz);
            dataMax.resize(sz);
            dataAvg.resize(sz);
        }
    }
    lowfreq = pFFTHelp->GetLowFreqHz() / 1000000.0;
    highfreq = pFFTHelp->GetHighFreqHz() / 1000000.0;

    pFFTHelp->Lock(); // lock the data to resolve the update issues
    float *maxdat = pFFTHelp->MaxValues();// get max high-water marks
    float *mindat = pFFTHelp->MinValues();// get max high-water marks
    float *avdat = pFFTHelp->AvgData();// get average values
    float *noisefloor = pFFTHelp->GetNoiseFloor();
    bool showAvg = GraphVisible(FFT_AVG_GRAPH_NAME);

    pFFTHelp->CalcNoiseFloor();

    for (int c = 0; c < pFFTHelp->GetBinSize(); c++)
    {
        double yaxfreqMhz = pFFTHelp->GetFreqHz(c);
        yaxfreqMhz /= 1000000.0; // convert from Hz to Mhz
        freqs[c] = yaxfreqMhz;
        datanoisefloor[c] = noisefloor[c];
        dataMax[c] = maxdat[c];
        if(showAvg)
        {
            dataAvg[c] = avdat[c];
        }
    }
    pFFTHelp->Unlock(); // unlock it so it can be updated again

    QCPGraph *gr = GetGraph(FFT_GRAPH_NAME);
  //  if(gr)
        gr->setData(freqs,datanoisefloor);

  //  if(showAvg)
    {
        QCPGraph *avgr = GetGraph(FFT_AVG_GRAPH_NAME);
        if(avgr)
            avgr->setData(freqs,dataAvg);
    }

    bool showMax = GraphVisible(FFT_MAX_GRAPH_NAME);
    if(showMax)
    {
        QCPGraph *grmax = GetGraph(FFT_MAX_GRAPH_NAME);
        if(grmax)
            grmax->setData(freqs,dataMax);
    }


    double lower = -140;
    double upper = -30;

    //autorange = true;
    if(autorange)
    {
        //get the right max val
        if(showMax)
        {
            for(int c = 0; c < pFFTHelp->GetBinSize(); c++)
            {
                if(c == 0 )
                {
                    upper = maxdat[0];
                    lower = -120;//mindat[5];
                }else
                {
                    if(maxdat[c] > upper) upper = maxdat[c];
                    //if(mindat[c] < lower && c > 5) lower = mindat[c];
                }
            }
        }else
        {
            upper = pFFTHelp->GetMaxDBM();
            lower = pFFTHelp->GetMinDBM();
        }


    }else
    {
        lower = rangelowY;
        upper = rangehighY;
    }
    plot->yAxis->setRange(lower, upper);
    plot->yAxis2->setRange(lower, upper);
}
