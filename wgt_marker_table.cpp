#include "wgt_marker_table.h"
#include "ui_wgt_marker_table.h"
#include <QScrollBar>
#include <QMessageBox>

wgt_marker_table::wgt_marker_table(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::wgt_marker_table)
{
    ui->setupUi(this);
    _selected = 0;
    updategui = false;
    UpdateButtons();
}

wgt_marker_table::~wgt_marker_table()
{
    delete ui;
}

void wgt_marker_table::SetMarkers(freq_markers *markers)
{
    _markers = markers;
    connect(_markers,SIGNAL(MarkerAdded(ftmarker*)),this,SLOT(onMarkerAdded(ftmarker*)));
    connect(_markers,SIGNAL(MarkersAdded(QVector<ftmarker *>)),this,SLOT(onMarkersAdded(QVector<ftmarker *>)));

    connect(_markers,SIGNAL(MarkerChanged(ftmarker*)),this,SLOT(onMarkerChanged(ftmarker*)));
    connect(_markers,SIGNAL(MarkerRemoved(ftmarker*)),this,SLOT(onMarkerRemoved(ftmarker*)));
    UpdateTable();
}

/*
This is called from the main form to set the select mark
*/
void wgt_marker_table::SetSelected(ftmarker *selected)
{
    _selected  = selected;// set it selected
    ui->wgt_marker_edit->SetMarker(_selected); // update the editor view
    //update the status of the buttons
    UpdateButtons();
    //select it on the table
    updategui = true;
    SelectMarkerOnTable(selected);
    // make sure the table doesn't emit events when changing
    updategui = false;
}

void wgt_marker_table::SelectMarkerOnTable(ftmarker *marker)
{
    int idx = 0;
    for(idx = 0; idx < _markers->m_markers.size(); idx ++)
    {
        ftmarker *ftm = _markers->m_markers[idx];
        if(ftm == marker)
        {
            ui->tblMarkers->selectRow(idx);
            break;
        }
    }
}

void wgt_marker_table::UpdateTable()
{
   ui->tblMarkers->clear(); // clear all previous
   QStringList labels;

   // set number of columns
   ui->tblMarkers->setColumnCount(4); // 5
   //set number of rows
   ui->tblMarkers->setRowCount(_markers->m_markers.size());
    labels << tr("Name") << tr("Freq") << tr("BW") << tr("Tags");//  << tr("Demod")  ;
   ui->tblMarkers->setHorizontalHeaderLabels(labels);

   int cnt = 0;
   int idx =0;
   for(cnt = 0; cnt < _markers->m_markers.size() ; cnt++)
   {
       idx = 0;
       ftmarker * fm = _markers->m_markers.at(cnt);

       QTableWidgetItem* nameitem = new QTableWidgetItem();
       nameitem->setText(fm->Name());
       ui->tblMarkers->setItem(cnt, idx++, nameitem );

       QTableWidgetItem* freqitem = new QTableWidgetItem();
       float freqMhz = fm->CF_MHz();
       freqitem->setText(QString::number(freqMhz));
       ui->tblMarkers->setItem(cnt, idx++, freqitem );

       QTableWidgetItem* bwitem = new QTableWidgetItem();
       QString bwtxt = QString::number(fm->BW_MHz());
       bwitem->setText(bwtxt);
       ui->tblMarkers->setItem(cnt, idx++, bwitem);

       QTableWidgetItem* tagitem = new QTableWidgetItem();
       tagitem->setText(fm->Tags());
       ui->tblMarkers->setItem(cnt, idx++, tagitem);

   }

   ui->tblMarkers->verticalScrollBar()->setStyleSheet("QScrollBar:vertical { width: 35px; }");
   ui->tblMarkers->resizeColumnsToContents();
}

void wgt_marker_table::UpdateButtons()
{
    if(_selected)
    {
        ui->cmdExport->setEnabled(true);
        ui->cmdGoto->setEnabled(true);
        ui->cmdRemove->setEnabled(true);
    }else
    {
        ui->cmdExport->setEnabled(false);
        ui->cmdGoto->setEnabled(false);
        ui->cmdRemove->setEnabled(false);
    }
}

void wgt_marker_table::onMarkerChanged(ftmarker *mrk)
{
    Q_UNUSED(mrk)
    UpdateTable();
}

void wgt_marker_table::onMarkerAdded(ftmarker *mrk)
{
    Q_UNUSED(mrk)
    UpdateTable();
    SetSelected(mrk);
}

void wgt_marker_table::onMarkersAdded(QVector<ftmarker *> markers)
{
    //Q_UNUSED(mrk)
    UpdateTable();
    //SetSelected(mrk);
}

void wgt_marker_table::onMarkerRemoved(ftmarker *mrk)
{
    Q_UNUSED(mrk)
    UpdateTable();
}

void wgt_marker_table::on_tblMarkers_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    if(updategui)
        return;
    Q_UNUSED(current)
    Q_UNUSED(previous)
    //crashes while deleting markers
    //get the current row
    int row = ui->tblMarkers->currentRow();
    if(row == -1)return;
    if(row >  _markers->m_markers.size())
        return;

    //get a pointer to the singular selected row
    ftmarker *mrk = _markers->m_markers.at(row);
    //show that info in the marker editor
    ui->wgt_marker_edit->SetMarker(mrk);
    _selected = mrk; // set the selected
    emit(onMarkerHighlight(mrk));
    UpdateButtons();
}

void wgt_marker_table::on_cmdRemove_clicked()
{
    //now remove it from the list
    if(_selected)
    {
        _markers->RemoveMarker(_selected);
        _selected = 0;
        ui->wgt_marker_edit->SetMarker(0);
        UpdateButtons();
        UpdateTable();
    }
}

void wgt_marker_table::on_cmdAdd_clicked()
{
    emit(onAddMarker());
}

void wgt_marker_table::on_cmdGoto_clicked()
{
    if(_selected)
    {
        emit(onMarkerSelected(_selected));
    }
}

void wgt_marker_table::on_cmdExport_clicked()
{
    if(_selected)
    {
        emit(onExportMarker(_selected));
    }
}

void wgt_marker_table::on_cmdRemoveAll_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Remove All", "This will remove all markers, Confirm?",
                            QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        emit(onRemoveAllMarkers());
        UpdateTable();
    }
}

void wgt_marker_table::on_cmdMerge_clicked()
{
    emit(onMerge());
}

void wgt_marker_table::on_cmdDirectionFind_clicked()
{

    //get the current sleected marker
    //get the current row
    int row = ui->tblMarkers->currentRow();
    if(row == -1)return;
    if(row >  _markers->m_markers.size())
        return;

    //get a pointer to the singular selected row
    ftmarker *mrk = _markers->m_markers.at(row);
    //raise a signal to tell the main form we're starting a direction find
    emit(onStartDF(mrk));

}
