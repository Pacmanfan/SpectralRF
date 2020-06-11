#ifndef RADIOSETTINGS_H
#define RADIOSETTINGS_H

#include <QDialog>
#include <sdr_device_manager.h>


namespace Ui {
class RadioSettings;
}

class RadioSettings : public QDialog
{
    Q_OBJECT

public:
    explicit RadioSettings(QWidget *parent = 0);
    ~RadioSettings();
    void Setup(SDR_Device_Manager *devs);
    SDR_Device_Manager *m_devs;
    void ListRadios();

private slots:
    void on_lstradios_currentRowChanged(int currentRow);

private:
    Ui::RadioSettings *ui;
};

#endif // RADIOSETTINGS_H
