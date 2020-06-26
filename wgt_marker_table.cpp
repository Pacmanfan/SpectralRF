#include "wgt_marker_table.h"
#include "ui_wgt_marker_table.h"
#include <QScrollBar>
#include <QMessageBox>
#include <QCheckBox>
bool checkchanging = false;

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
    connect(_markers,SIGNAL(MarkersCleared()),this,SLOT(onMarkersCleared()));
    connect(_markers,SIGNAL(MarkerSelected(ftmarker*)),this,SLOT(onMarkerSelected(ftmarker*)));
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

void wgt_marker_table::addCheckBoxAt(int row_number, int column_number,bool state)
{

    // Create a widget that will contain a checkbox
 //    QWidget *checkBoxWidget = new QWidget();
     QCheckBox *checkBox = new QCheckBox();      // We declare and initialize the checkbox
   //  QHBoxLayout *layoutCheckBox = new QHBoxLayout(checkBoxWidget); // create a layer with reference to the widget
   //  layoutCheckBox->addWidget(checkBox);            // Set the checkbox in the layer
   //  layoutCheckBox->setAlignment(Qt::AlignCenter);  // Center the checkbox
   //  layoutCheckBox->setContentsMargins(0,0,0,0);    // Set the zero padding
     /* Check on the status of odd if an odd device,
       * exhibiting state of the checkbox in the Checked, Unchecked otherwise
       * */
      checkBox->setChecked(state);

      //ui->tblMarkers->setCellWidget(row_number,column_number, checkBoxWidget);
      ui->tblMarkers->setCellWidget(row_number,column_number, checkBox);

    connect(checkBox,SIGNAL(stateChanged(int)),this,SLOT(onCheckboxState(int)));
     // Another way to add check box as item
    /*

   // QTableWidgetItem *checkBoxItem = new QTableWidgetItem("checkbox string ");
    QTableWidgetItem *checkBoxItem = new QTableWidgetItem();
    checkBoxItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    checkBoxItem->setCheckState(Qt::Checked);
    ui->job_table_view->setItem(row_number,column_number,checkBoxItem);

    */
}

void wgt_marker_table::UpdateTable()
{
    if(checkchanging)
        return;
   ui->tblMarkers->clear(); // clear all previous
   QStringList labels;

   // set number of columns
   ui->tblMarkers->setColumnCount(4); // 5
   //set number of rows
   ui->tblMarkers->setRowCount(_markers->m_markers.size());
    labels << tr("Visible")<< tr("Name") << tr("Freq") << tr("BW") << tr("Tags");//  << tr("Demod")  ;
   ui->tblMarkers->setHorizontalHeaderLabels(labels);

   int cnt = 0;
   int idx =0;
   for(cnt = 0; cnt < _markers->m_markers.size() ; cnt++)
   {
       idx = 0;
       ftmarker * fm = _markers->m_markers.at(cnt);
        addCheckBoxAt(cnt,idx++,fm->visible());

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
        ui->cmdRemove->setEnabled(true);
    }else
    {
        ui->cmdRemove->setEnabled(false);
    }
}

void wgt_marker_table::onMarkerChanged(ftmarker *mrk)
{
    Q_UNUSED(mrk)
    UpdateTable();
    ui->wgt_marker_edit->SetMarker(mrk);
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
}

void wgt_marker_table::onMarkerRemoved(ftmarker *mrk)
{
    Q_UNUSED(mrk)
    UpdateTable();
}

void wgt_marker_table::onMarkersCleared()
{
    // clear the table view
    UpdateTable();
    ui->wgt_marker_edit->SetMarker(nullptr);
    _selected = nullptr; // set the selected as nothing
}

/*
Signal received that a marker was selected, ignore it if we're the one who just did it
*/
void wgt_marker_table::onMarkerSelected(ftmarker *mrk)
{
    if(updategui)
        return;
    //select the item on the table
    SelectMarkerOnTable(mrk);
    ui->wgt_marker_edit->SetMarker(mrk);
    _selected = mrk; // set the selected
}

void wgt_marker_table::onCheckboxState(int state)
{
    // one or more of the visiblity checkboxes in the table has changed state
    // iterate throug hthe table, using the index and apply the new visibilty flag
    // to each marker
  //  int a= 100;
  //  a = 10; // test
    checkchanging = true;
    for(int c = 0 ; c <ui->tblMarkers->rowCount(); c++)
    {
        QCheckBox * cb = (QCheckBox *)ui->tblMarkers->cellWidget(c,0);
        ftmarker * fm = _markers->m_markers.at(c);
        fm->setVisible(cb->isChecked());
    }
    checkchanging = false;
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
    _markers->Select(mrk);
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
    emit(onAddMarker()); // request for the main window to add the marker
    // alternatively, the fft window could do it because it has the ftmarker_GUI object
}

void wgt_marker_table::on_cmdGoto_clicked()
{
    if(_selected)
    {
        emit(onMarkerSelected(_selected));
    }
}


void wgt_marker_table::on_cmdRemoveAll_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Remove All", "This will remove all markers, Confirm?",
                            QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        //emit(onRemoveAllMarkers());
        _markers->Clear();
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

void wgt_marker_table::on_cmdLoadMarkers_clicked()
{
    emit(onLoadMarkers()); // done in the main window
}

void wgt_marker_table::on_cmdSaveMarkers_clicked()
{
    emit(onSaveMarkers()); // done in the main window
}
