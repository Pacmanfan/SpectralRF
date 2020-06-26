#include "wgt_waterfall.h"
#include "ui_wgt_waterfall.h"

wgt_waterfall::wgt_waterfall(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::wgt_waterfall)
{
    ui->setupUi(this);
    Initialize();
    SetControlsVisible(false);
}

wgt_waterfall::~wgt_waterfall()
{
    delete ui;
}


void wgt_waterfall::Initialize()
{

    ctrlpressed = false;
    plot = ui->wgtWaterfall;

    // configure axis rect:
    plot->axisRect()->setupFullAxesBox(true);

    plot->axisRect()->setAutoMargins(QCP::MarginSides());
    //plotFFT->axisRect()->setMargins(QMargins(29,1,12,1));
    plot->axisRect()->setMargins(QMargins(70,1,12,1));

    plot->yAxis->setVisible(true);
    plot->yAxis->setLabel("Time (Seconds)");
    plot->yAxis->setNumberPrecision(4);

    plot->axisRect()->setBackground(Qt::black);
    // set up the QCPColorMap:
    colorMap = new QCPColorMap(plot->xAxis, plot->yAxis);

    colorMap->setInterpolate(false);
    plot->setInteractions( QCP::iRangeZoom | QCP::iRangeDrag);
    //allow us to select rectangles
    plot->setInteraction(QCP::iSelectPlottables, true);
    plot->axisRect()->setRangeZoomFactor(1.25,1);
    plot->axisRect()->setRangeDrag(Qt::Horizontal);

    // mouse move
    connect(plot, SIGNAL(mouseMove(QMouseEvent*)), this,
        SLOT(OnMouseMove(QMouseEvent*)));

    //SINGLE MOUSE CLICK (used for INPUTING threshold)
    connect(plot, SIGNAL(mousePress(QMouseEvent*)), this,
            SLOT(OnMousePressed(QMouseEvent*)));

    //Mouse Release
    connect(plot, SIGNAL(mouseRelease(QMouseEvent*)), this,
            SLOT(OnMouseRelease(QMouseEvent*)));

    connect(plot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this,
            SLOT(OnMouseDoubleClick(QMouseEvent*)));

    connect(plot,SIGNAL(itemDoubleClick(QCPAbstractItem*,QMouseEvent*)),this,SLOT(onitemDoubleClick(QCPAbstractItem*,QMouseEvent*)));
    connect(plot,SIGNAL(itemClick(QCPAbstractItem*,QMouseEvent*)),this,SLOT(onitemClick(QCPAbstractItem*,QMouseEvent*)));


    //also set up click focus for key events
    plot->setFocusPolicy(Qt::ClickFocus);
    //capture key presses
    plot->installEventFilter(this);

    m_rectselect = new ftmarker(); // new marker for rectangular selection
    m_selector = new ftmarker_GUI(m_rectselect,plot); // the rectangular selector
    m_selector->m_rect->setPen(QPen(Qt::blue,VLINEWIDTH));
    m_selector->SetVisible(false);

}

bool wgt_waterfall::getWaterfallDirection() const
{
    return WaterfallDirection;
}

void wgt_waterfall::setWaterfallDirection(bool value)
{
    WaterfallDirection = value;
}

void wgt_waterfall::SetControlsVisible(bool val)
{
    ui->wgtControls->setVisible(val);;
}


bool wgt_waterfall::getAutorange() const
{
    return autorange;
}

void wgt_waterfall::setAutorange(bool value)
{
    autorange = value;
}

void wgt_waterfall::setXRange(double low, double high)
{
    plot->xAxis->setRange(low,high);
}

void wgt_waterfall::Update(FFT_Hist *pFFTHelp, double tlow, double thigh)
{


    //SET UP SOME MIN/MAX's we can use for GUI WINDOW.
    double minFreq=0;
    double maxFreq=0;


    lowfreq = pFFTHelp->GetLowFreqHz() / 1000000;
    highfreq = pFFTHelp->GetHighFreqHz() / 1000000;

    int nx = pFFTHelp->GetBinSize();
    int ny = pFFTHelp->MaxRows();// find out how many row entries
    if(nx ==0 || ny ==0)
        return;

    pFFTHelp->Lock();

    colorMap->data()->setSize(nx, ny);//mSWTFFileListDepth); // we want the color map to have nx * ny data points

    float *dp = colorMap->data()->GetDataPtr();

    double lower = -150;
    double upper = -30;
    if(autorange)
    {
        lower = pFFTHelp->GetMinDBM();
        upper = pFFTHelp->GetMaxDBM();
    }else
    {
        lower = waterlow;
        upper = waterhigh;
    }

    pFFTHelp->CopyTo(dp);
    //float *cdat = pFFTHelp->GetRow(0);
    //memcpy(dp,cdat,nx*ny*sizeof(float));


    minFreq = pFFTHelp->GetFreqHz(0);
    maxFreq = pFFTHelp->GetFreqHz(pFFTHelp->GetBinSize() -1);
    minFreq /= 1000000;
    maxFreq /= 1000000;

    colorMap->data()->setRange(QCPRange(minFreq, maxFreq), QCPRange(tlow,thigh)); // need a time scaler here too
    plot->yAxis->setRange(tlow,thigh);

    plot->yAxis->setRangeReversed(WaterfallDirection);

    colorMap->data()->SetDataBounds(lower,upper);
    colorMap->setDataRange(QCPRange(lower,upper));
    pFFTHelp->Unlock();
}

void wgt_waterfall::AddTuner(ftmarker *tunermarker)
{
    sigtuner_GUI *stg = new sigtuner_GUI(tunermarker,plot);
    m_tuners_gui.append(stg); // add it to the list for safekeeping

}

void wgt_waterfall::UnselectMarkers()
{
    for(int c= 0; c< m_markers_gui.size(); c++)
    {
        ftmarker_GUI *mrk = m_markers_gui.at(c);
        mrk->SetSelected(false);
    }
}

void wgt_waterfall::SetMarkerSelected(ftmarker *ftm)
{    
    for(int c= 0; c< m_markers_gui.size(); c++)
    {
        ftmarker_GUI *mrk = m_markers_gui.at(c);
        if(ftm == mrk->m_marker)
        {
            mrk->SetVisible(true); // make sure it's visible
            mrk->SetSelected(true); // set it selected
        }
    }
}

void wgt_waterfall::SetMarkersSelected(QVector<ftmarker *> markers)
{
    for(int c = 0 ; c< markers.size(); c++)
    {
        SetMarkerSelected(markers[c]);
    }
}


void wgt_waterfall::OnMouseRelease(QMouseEvent *evt)
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

    if(ctrlpressed)
    {
        // iterate through all markers,
        //determine a list of markers that intersect with the waterfall selection marker
        // raise an event to notify the rest of the app

        QVector<ftmarker *> mrks = m_markers->Intersects(m_rectselect);
        if(mrks.size() > 0)
        {
            m_selector->SetVisible(false); // since we actually made a selection, make it invisible
        }
        //send even empty sets
        UnselectMarkers();
        SetMarkersSelected(mrks);
       // emit(OnMarkersSelected(mrks));

    }

    miWaterfall.rb = false;
    miWaterfall.lb = false;
}

void wgt_waterfall::OnMouseMove(QMouseEvent *evt)
{
    Q_UNUSED(evt);

    miWaterfall.Set(evt->pos().x(),evt->pos().y());
    // update the phase tracer position
    float x = plot->xAxis->pixelToCoord(evt->pos().x());
    emit(OnFreqHighlight(x));
    //check and see if we've selected the threshold


    float xp = evt->pos().x();
    float xv = plot->xAxis->pixelToCoord(xp);

    float yp = evt->pos().y();
    float yv = plot->yAxis->pixelToCoord(yp);

    emit(OnTimeHighlight(yv));
    bool moved = false;

    if(ctrlpressed == true && miWaterfall.lb == true)
    {
        //we're moving the selector here
        m_rectselect->setFreqHighMHz(xv);
        m_rectselect->setEndTime_S(yv);
    }
    else
    {

        for(int c= 0; c < m_tuners_gui.size(); c++ )
        {
            sigtuner_GUI *stg = m_tuners_gui[c];

            float bw = stg->m_tunermarker->BW_MHz();
            float nbw = 0;
            float cf = stg->m_tunermarker->CF_MHz();
            switch(stg->m_ms)
            {
                case eMV_None:
                    // we're not mving anything , so get the y diff
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
                    moved = true;
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
                    moved = true;
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
                    moved = true;
                    break;
            }

        }

    } // end if
    if(miWaterfall.lb && !moved && !ctrlpressed)
    {
        double ydiff = miWaterfall.ly - miWaterfall.my;
        emit(Pan(ydiff));
    }
}




void wgt_waterfall::OnMousePressed(QMouseEvent *evt)
{
    miWaterfall.Set(evt->pos().x(),evt->pos().y());

    float xv = plot->xAxis->pixelToCoord(evt->pos().x());
    float yv = plot->yAxis->pixelToCoord(evt->pos().y());
    if(evt->button() == Qt::RightButton)
    {
        miWaterfall.rb = true;
    }
    if(evt->button() == Qt::LeftButton)
    {
        miWaterfall.lb = true;

        if(ctrlpressed == true) // we're moving the selector
        {
            // set the upper left coordinate of the selector
            m_rectselect->setFreqLowMHz(xv);
            m_rectselect->setStartTime_S(yv);
            m_rectselect->setFreqHighMHz(xv);
            m_rectselect->setEndTime_S(yv);
            m_selector->SetVisible(true);
        }
        else
        {

            for(int c= 0; c < m_tuners_gui.size(); c++ )
            {
                sigtuner_GUI *stg = m_tuners_gui[c];
                int xp;
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
                // check to see if it's selected
                xp = stg->CenterFreqLineFFT->point1->pixelPosition().x();
                if(evt->pos().x()  > (xp - LINEGRIPWIDTH) && evt->pos().x() < (xp + LINEGRIPWIDTH))
                {
                   stg->m_ms = eMV_CenterF;
                   stg->CenterFreqLineFFT->setSelected(true);
                   plot->setInteraction ( QCP::iRangeDrag, false ); // turn off range drag for now
                }
            }

        }
    }

}

void wgt_waterfall::OnMouseDoubleClick(QMouseEvent *evt)
{
    Q_UNUSED(evt)
    //de-select and hide the waterfall rect selector
    m_selector->SetVisible(false);
}


bool wgt_waterfall::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Control)
        {
            ctrlpressed = true;
            plot->setInteraction( QCP::iRangeDrag, false);
        }
        if(keyEvent->key() == Qt::Key_Delete)
        {
           // emit(OnDeleteCurrentMarker());
        }

    }else if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Control)
        {
            ctrlpressed = false;
            plot->setInteraction( QCP::iRangeDrag, true);
        }
    }
    return QObject::eventFilter(obj, event);
}

void wgt_waterfall::setMarkers(freq_markers *markers)
{
    m_markers = markers;
    //now, update the gui to reflect all the markers
    connect(m_markers,SIGNAL(MarkerAdded(ftmarker*)),this,SLOT(onMarkerAdded(ftmarker*)));
    connect(m_markers,SIGNAL(MarkerChanged(ftmarker*)),this,SLOT(onMarkerChanged(ftmarker*)));
    connect(m_markers,SIGNAL(MarkerRemoved(ftmarker*)),this,SLOT(onMarkerRemoved(ftmarker*)));
    connect(m_markers,SIGNAL(MarkersAdded(QVector<ftmarker*>)),this,SLOT(onMarkersAdded(QVector<ftmarker*>)));
    connect(m_markers,SIGNAL(MarkersCleared()),this,SLOT(onMarkersCleared()));
    connect(m_markers,SIGNAL(MarkerSelected(ftmarker*)),this,SLOT(onMarkerSelected(ftmarker*)));
}

void wgt_waterfall::onitemDoubleClick(QCPAbstractItem *item ,QMouseEvent *ev)
{
    Q_UNUSED(ev)
    UnselectMarkers(); // unselect any previously selected markers
    m_selector->SetVisible(false);

    for(int c= 0; c< m_markers_gui.size(); c++)
    {
        ftmarker_GUI *mrk = m_markers_gui.at(c);
        if((void *)item == (void *)mrk->m_rect)
        {
            m_markers->Select(mrk->m_marker);
            mrk->SetSelected(true); // set it selected on screen
            break;
        }
    }
}

/*
This is the item 'soft select', it should select the item, and select it on the markers screen
*/

void wgt_waterfall::onitemClick(QCPAbstractItem *item ,QMouseEvent *ev)
{
    Q_UNUSED(ev)
    UnselectMarkers(); // unselect any previously selected markers
    m_selector->SetVisible(false);

    for(int c= 0; c< m_markers_gui.size(); c++)
    {
        ftmarker_GUI *mrk = m_markers_gui.at(c);
        if((void *)item == (void *)mrk->m_rect)
        {
            //emit(OnMarkerSelected(mrk->m_marker,true));
            m_markers->Select(mrk->m_marker);
            mrk->SetSelected(true); // set it selected on screen
            break;
        }
    }
}

void wgt_waterfall::onMarkerChanged(ftmarker *mrk)
{
 // the ftmarker_GUI itself listens for a change in the marker
}

void wgt_waterfall::onMarkerAdded(ftmarker *mrk)
{
    ftmarker_GUI *ftmg = new ftmarker_GUI(mrk,plot);
    ftmg->SetVisible(true);
    m_markers_gui.append(ftmg);
}

//done usually through a load
void wgt_waterfall::onMarkersAdded(QVector<ftmarker *> markers)
{
    for(int c = 0; c < markers.size(); c ++)
    {
        ftmarker_GUI *ftmg = new ftmarker_GUI(markers[c],plot);
        ftmg->SetVisible(true);
        m_markers_gui.append(ftmg);
    }
}

void wgt_waterfall::onMarkerRemoved(ftmarker *ftm)
{
    m_selector->SetVisible(false);
    int idx = -1;
    ftmarker_GUI *ftmg = 0;
    for(int c =0; c< m_markers_gui.size(); c++)
    {
        ftmg = m_markers_gui[c];
        if (ftmg->m_marker == ftm)
        {
            idx = c;
            break;
        }
    }
    if(idx != -1)
    {
        ftmg->SetVisible(false);
        m_markers_gui.removeAt(idx);
        delete ftmg;
    }
}

void wgt_waterfall::onMarkersCleared()
{
    for(int c =0; c< m_markers_gui.size(); c++)
    {
        ftmarker_GUI *ftmg = m_markers_gui[c];
        ftmg->SetVisible(false);
        delete ftmg;
    }
    m_markers_gui.clear();
    m_selector->SetVisible(false);
}

void wgt_waterfall::onMarkerSelected(ftmarker *mark)
{    
    UnselectMarkers();
    for(int c= 0; c< m_markers_gui.size(); c++)
    {
        ftmarker_GUI *mrk = m_markers_gui.at(c);
        if(mark == mrk->m_marker)
        {
            mrk->SetSelected(true);
            break;
        }
    }
}
