#include "sigtuner_gui.h"
#include <wgt_util.h>


void sigtuner_GUI::SetupLine(QCPItemStraightLine* line, float xv, QColor color)
{
    Q_UNUSED(m_ParentPlot)

    line->point1->setType(QCPItemPosition::ptPlotCoords);
    line->point2->setType(QCPItemPosition::ptPlotCoords);
    if(m_ParentPlot->graphCount() != 0)
    {
        line->point1->setAxes(m_ParentPlot->graph(0)->keyAxis(), m_ParentPlot->graph(0)->valueAxis());
        line->point2->setAxes(m_ParentPlot->graph(0)->keyAxis(), m_ParentPlot->graph(0)->valueAxis());
    }

    line->point1->setCoords(xv, 0);
    line->point2->setCoords(xv, -150);
    line->setPen(QPen(color,VLINEWIDTH));
    line->setSelectable(true);
}

void sigtuner_GUI::OnMarkerChanged(ftmarker *mrk)
{
    SetupLine(CenterFreqLineFFT,mrk->CF_MHz(),Qt::yellow);
    SetupLine(lowFreqLineFFT,mrk->FreqLowMHz(),Qt::green);
    SetupLine(highFreqLineFFT,mrk->FreqHighMHz(),Qt::green);
}

sigtuner_GUI::sigtuner_GUI(ftmarker *tunermarker, QCustomPlot *ParentPlot)
{
    m_ms = eMV_None;
    m_tunermarker = tunermarker;
    //listen for changes that occur elsewhere
    connect(m_tunermarker,SIGNAL(MarkerChanged(ftmarker*)),this,SLOT(OnMarkerChanged(ftmarker*)));
    m_ParentPlot = ParentPlot;
    lowFreqLineFFT = new QCPItemStraightLine(m_ParentPlot);
    highFreqLineFFT = new QCPItemStraightLine(m_ParentPlot);
    CenterFreqLineFFT = new QCPItemStraightLine(m_ParentPlot);

    SetupLine(CenterFreqLineFFT,m_tunermarker->CF_MHz(),Qt::yellow);
    SetupLine(lowFreqLineFFT,m_tunermarker->FreqLowMHz(),Qt::green);
    SetupLine(highFreqLineFFT,m_tunermarker->FreqHighMHz(),Qt::green);
}

