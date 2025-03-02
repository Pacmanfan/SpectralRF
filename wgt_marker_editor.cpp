#include "wgt_marker_editor.h"
#include "ui_wgt_marker_editor.h"

using namespace std;

wgt_marker_editor::wgt_marker_editor(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::wgt_marker_editor)
{
    ui->setupUi(this);
    m_marker = 0;
}

wgt_marker_editor::~wgt_marker_editor()
{
    delete ui;
}

void wgt_marker_editor::SetMarker(ftmarker *marker)
{
    m_marker = marker;
    UpdateUI();
}

void wgt_marker_editor::UpdateUI()
{
    if(m_marker != 0)
    {
        double BW = m_marker->FreqHighMHz() - m_marker->FreqLowMHz();
        ui->leName->setText(m_marker->Name());
        ui->leBW->setText(QString::number(BW));
        ui->leCF->setText(QString::number(m_marker->CF_MHz()));
        ui->leLowF->setText(QString::number(m_marker->FreqLowMHz()));
        ui->leHighF->setText(QString::number(m_marker->FreqHighMHz()));
        ui->leStartTime->setText(QString::number(m_marker->StartTime_S()));
        ui->leEndTime->setText(QString::number(m_marker->EndTime_S()));
        ui->chkNoStartT->setChecked(!m_marker->HasStartTime());
        ui->chkNoEndT->setChecked(!m_marker->HasEndTime());
        ui->leStartTime->setEnabled(m_marker->HasStartTime());
        ui->leEndTime->setEnabled(m_marker->HasEndTime());
        ui->leTags->setText(m_marker->Tags());
        double duration = m_marker->Duration_S();//Sel_highT - m_marker->Sel_lowT;
        if(m_marker->HasStartTime() && m_marker->HasEndTime())
        {
            ui->lblDuration->setText("Duration (S) : " + QString::number(duration,'f',6));
        }else
        {
            ui->lblDuration->setText("Duration (S) : N/A");
        }
    }
    else // clear the UI
    {
        ui->leName->setText("");
        ui->leBW->setText("");
        ui->leCF->setText("");
        ui->leLowF->setText("");
        ui->leHighF->setText("");
        ui->leStartTime->setText("");
        ui->leEndTime->setText("");
        ui->chkNoStartT->setChecked(false);
        ui->chkNoEndT->setChecked(false);
        ui->leStartTime->setEnabled(true);
        ui->leEndTime->setEnabled(true);
        ui->lblDuration->setText("");
    }
}

void wgt_marker_editor::on_chkNoStartT_clicked()
{
    if(!m_marker)return;
    m_marker->setHasStartTime(!ui->chkNoStartT->isChecked());
    UpdateUI();
    emit(MarkerChanged(m_marker));
}

void wgt_marker_editor::on_chkNoEndT_clicked()
{
    if(!m_marker)return;
    m_marker->setHasEndTime(!ui->chkNoEndT->isChecked());
    UpdateUI();
    emit(MarkerChanged(m_marker));
}

void wgt_marker_editor::on_leCF_editingFinished()
{
    if(!m_marker)return;
    bool ok =false;
    QString arg1 = ui->leCF->text();
    double val = arg1.toDouble(&ok);
    if(ok)
    {
        m_marker->setCF_MHz(val);
        UpdateUI();
        emit(MarkerChanged(m_marker));
    }
}

void wgt_marker_editor::on_leBW_editingFinished()
{
    if(!m_marker)return;
    bool ok =false;
    QString arg1 = ui->leBW->text();
    double val = arg1.toDouble(&ok);
    if(ok)
    {
        m_marker->setBW_MHz(val);
        UpdateUI();
        emit(MarkerChanged(m_marker));
    }
}

void wgt_marker_editor::on_leLowF_editingFinished()
{
    if(!m_marker)return;
    bool ok =false;
    QString arg1 = ui->leLowF->text();
    double val = arg1.toDouble(&ok);
    if(ok)
    {
        m_marker->setFreqLowMHz(val);
        if(m_marker->FreqHighMHz() < m_marker->FreqLowMHz())
        {
            double th,tl;
            th = m_marker->FreqHighMHz();
            tl = m_marker->FreqLowMHz();
            m_marker->setFreqHighMHz(tl);
            m_marker->setFreqLowMHz(th);
        }
        UpdateUI();
        emit(MarkerChanged(m_marker));
    }
}

void wgt_marker_editor::on_leHighF_editingFinished()
{
    if(!m_marker)return;
    bool ok =false;
    QString arg1 = ui->leHighF->text();
    double val = arg1.toDouble(&ok);
    if(ok)
    {
        m_marker->setFreqHighMHz(val);
        if(m_marker->FreqHighMHz() < m_marker->FreqLowMHz())
        {
            double th,tl;
            th = m_marker->FreqHighMHz();
            tl = m_marker->FreqLowMHz();
            m_marker->setFreqHighMHz(tl);
            m_marker->setFreqLowMHz(th);
        }
        UpdateUI();
        emit(MarkerChanged(m_marker));
    }
}

void wgt_marker_editor::on_leStartTime_editingFinished()
{
    if(!m_marker)return;
    bool ok =false;
    QString arg1 = ui->leStartTime->text();
    double val = arg1.toDouble(&ok);
    if(ok)
    {
        m_marker->setStartTime_S(val);
        UpdateUI();
        emit(MarkerChanged(m_marker));
    }
}

void wgt_marker_editor::on_leEndTime_editingFinished()
{
    if(!m_marker)return;
    bool ok =false;
    QString arg1 = ui->leEndTime->text();
    double val = arg1.toDouble(&ok);
    if(ok)
    {
        m_marker->setEndTime_S(val);
        emit(MarkerChanged(m_marker));
    }
}

void wgt_marker_editor::on_leName_editingFinished()
{
    if(!m_marker)return;
    m_marker->setName(ui->leName->text());
    emit(MarkerChanged(m_marker));
}

void wgt_marker_editor::on_leTags_editingFinished()
{
     if(!m_marker)return;
     m_marker->setTags(ui->leTags->text());
     emit(MarkerChanged(m_marker));
}
