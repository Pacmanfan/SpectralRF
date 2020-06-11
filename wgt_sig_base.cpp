#include "wgt_sig_base.h"

wgt_Sig_Base::wgt_Sig_Base()
{

}

/*
Add a new trace to this graph to display
*/
QCPGraph *wgt_Sig_Base::AddGraph(QString tracename,QColor color)
{
    QCPGraph *graph = plot->addGraph(plot->xAxis);
    graph->setPen(QPen(color,1));
    graph->setName(QString(tracename));
    graphs.insert(tracename,graph);
    return graph;
}

QCPGraph *wgt_Sig_Base::AddGraph(QString tracename, QCPAxis *axis, QColor color)
{
    QCPGraph *graph = plot->addGraph(axis);
    graph->setPen(QPen(color,1));
    graph->setName(QString(tracename));
    graphs.insert(tracename,graph);
    return graph;
}

/*
Return a trace for direct manipulation
*/
QCPGraph *wgt_Sig_Base::GetGraph(QString tracename)
{
    if(graphs.contains(tracename))
    {
        return graphs[tracename];
    }
    return 0;
}
/*
remove a trace if present
*/
void wgt_Sig_Base::RemoveGraph(QString tracename)
{
    if(graphs.contains(tracename))
    {
        graphs.remove(tracename);
    }
}

bool wgt_Sig_Base::GraphVisible(QString tracename)
{
    QCPGraph *graph = GetGraph(tracename);
    if(graph)
        return graph->visible();
    return false;
}

