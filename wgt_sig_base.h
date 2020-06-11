#ifndef WGT_SIG_BASE_H
#define WGT_SIG_BASE_H
#include <QMap>
#include <qcustomplot.h>
#include <wgt_util.h>
/*
Base class for signal displaying widgets
contains common management code

*/

class wgt_Sig_Base
{
public:
    wgt_Sig_Base();
    QCustomPlot *plot; // the plot
    QMap<QString, QCPGraph *> graphs;
    virtual QCPGraph *AddGraph(QString tracename,QColor color); // defaults to the xaxis
    virtual QCPGraph *AddGraph(QString tracename,QCPAxis *axis,QColor color); // add to a specific axis
    QCPGraph *GetGraph(QString tracename);
    virtual void RemoveGraph(QString tracename);
    bool GraphVisible(QString tracename);

};

#endif // WGT_SIG_BASE_H

