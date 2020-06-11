#include "ftmarker_gui.h"

ftmarker_GUI::ftmarker_GUI(ftmarker *marker, QCustomPlot *ParentPlot)
{
    m_marker = marker;
    connect(m_marker,SIGNAL(MarkerChanged(ftmarker*)),this,SLOT(OnMarkerChanged(ftmarker*)));
    m_rect = new QCPItemRect(ParentPlot);
    m_parent = ParentPlot;
    m_rect->setSelectable(true);
    m_rect->setPen(QPen(Qt::green,VLINEWIDTH));
    SetupRect();
}

ftmarker_GUI::~ftmarker_GUI()
{
    m_rect->setVisible(false);
   // delete m_rect;
}

void ftmarker_GUI::SetupRect()
{
    m_rect->topLeft->setType(QCPItemPosition::ptPlotCoords);
    m_rect->topLeft->setAxes(m_parent->xAxis, m_parent->yAxis);
    m_rect->bottomRight->setType(QCPItemPosition::ptPlotCoords);
    m_rect->bottomRight->setAxes(m_parent->xAxis, m_parent->yAxis);
    double lowt,hight;
    if(m_marker->HasStartTime())
    {
        lowt = m_marker->StartTime_S() ; // convert to ms?
    }else
    {
        lowt = 0;
    }
    if(m_marker->HasEndTime())
    {
        hight = m_marker->EndTime_S() ; // convert to ms?
    }else
    {
        hight = 1000000; // some high value
    }

    m_rect->topLeft->setCoords(m_marker->FreqLowMHz(),lowt);
    m_rect->bottomRight->setCoords(m_marker->FreqHighMHz(),hight);

}

void ftmarker_GUI::SetVisible(bool val)
{
    m_rect->setVisible(val);
}

bool ftmarker_GUI::IsVisible()
{
   return m_rect->visible();
}

void ftmarker_GUI::OnMarkerChanged(ftmarker *mrk)
{
    SetupRect();
}
