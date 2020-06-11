#ifndef FTMARKER_GUI_H
#define FTMARKER_GUI_H

#include <QObject>
#include <ftmarker.h>
#include <qcustomplot.h>
#include <wgt_util.h>

class ftmarker_GUI : public QObject
{
    Q_OBJECT
public:
    //explicit ftmarker_GUI(QObject *parent = 0);
    ftmarker_GUI(ftmarker *marker,QCustomPlot *ParentPlot);
    ~ftmarker_GUI();
    ftmarker *m_marker;
    QCPItemRect *m_rect;
    QCustomPlot *m_parent;
    void SetupRect();
    void SetVisible(bool val);
    bool IsVisible();
    void SetSelected(bool val){m_rect->setSelected(val);}
signals:

public slots:
private slots:
    void OnMarkerChanged(ftmarker *mrk);
};

#endif // FTMARKER_GUI_H
