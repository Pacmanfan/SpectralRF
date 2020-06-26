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
    void addCheckBoxAt(int row_number, int column_number,bool state);
private:
    Ui::wgt_marker_table *ui;
    freq_markers *_markers; // pointer reference to the marker table
    ftmarker *_selected;
    bool updategui;
public slots:
    void onMarkerChanged(ftmarker *mrk);
    void onMarkerAdded(ftmarker *mrk);
    void onMarkersAdded(QVector<ftmarker *> markers);
    void onMarkerRemoved(ftmarker *mrk);
    void onMarkersCleared();
    void onMarkerSelected(ftmarker *mrk);

    void onCheckboxState(int state);

signals:

    void onAddMarker();
    //void onRemoveMarker();
    void onMerge();


    void onStartDF(ftmarker *mrk);// start a direction find on this marker
    void onLoadMarkers();
    void onSaveMarkers();

private slots:
    void on_tblMarkers_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void on_cmdRemove_clicked();
    void on_cmdAdd_clicked();
    void on_cmdGoto_clicked();
    void on_cmdRemoveAll_clicked();
    void on_cmdMerge_clicked();
    void on_cmdDirectionFind_clicked();
    void on_cmdLoadMarkers_clicked();
    void on_cmdSaveMarkers_clicked();
};

#endif // WGT_MARKER_TABLE_H
