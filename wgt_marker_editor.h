#ifndef WGT_MARKER_EDITOR_H
#define WGT_MARKER_EDITOR_H

#include <QFrame>
#include <ftmarker.h>
namespace Ui {
class wgt_marker_editor;
}

class wgt_marker_editor : public QFrame
{
    Q_OBJECT

public:
    explicit wgt_marker_editor(QWidget *parent = 0);
    ~wgt_marker_editor();
    void SetMarker(ftmarker *marker);
    void UpdateUI(); // set to the screen
private slots:

    void on_chkNoStartT_clicked();
    void on_chkNoEndT_clicked();
    void on_leCF_editingFinished();
    void on_leBW_editingFinished();
    void on_leLowF_editingFinished();
    void on_leHighF_editingFinished();
    void on_leStartTime_editingFinished();
    void on_leEndTime_editingFinished();

    void on_leName_editingFinished();

    void on_leTags_editingFinished();

private:
    Ui::wgt_marker_editor *ui;
    ftmarker *m_marker;
signals:
    void MarkerChanged(ftmarker *marker);
};

#endif // WGT_MARKER_EDITOR_H
