#ifndef WGT_MARKER_TABLE_H
#define WGT_MARKER_TABLE_H

#include <QWidget>
#include <ftmarker.h>
#include <QTableWidgetItem>

namespace Ui {
class wgt_marker_table;
}

class wgt_marker_table : public QWidget
{
    Q_OBJECT

public:
    explicit wgt_marker_table(QWidget *parent = 0);
    ~wgt_marker_table();
    void SetMarkers(freq_markers *markers);
    void SetSelected(ftmarker *selected);
    void UpdateTable();
    void UpdateButtons();
    void SelectMarkerOnTable(ftmarker *marker);
private:
    Ui::wgt_marker_table *ui;
    freq_markers *_markers;
    ftmarker *_selected;
    bool updategui;
public slots:
    void onMarkerChanged(ftmarker *mrk);
    void onMarkerAdded(ftmarker *mrk);
    void onMarkersAdded(QVector<ftmarker *> markers);
    void onMarkerRemoved(ftmarker *mrk);
signals:
    void onMarkerSelected(ftmarker *mrk); // hard select - like a goto or double click
    void onMarkerHighlight(ftmarker *mrk); // soft select - like a table item change
    void onAddMarker();
    void onExportMarker(ftmarker *mrk);
    void onMerge();
    void onRemoveAllMarkers();
    void onStartDF(ftmarker *mrk);// start a direction find on this marker

private slots:
    void on_tblMarkers_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void on_cmdRemove_clicked();
    void on_cmdAdd_clicked();
    void on_cmdGoto_clicked();
    void on_cmdExport_clicked();
    void on_cmdRemoveAll_clicked();
    void on_cmdMerge_clicked();
    void on_cmdDirectionFind_clicked();
};

#endif // WGT_MARKER_TABLE_H
