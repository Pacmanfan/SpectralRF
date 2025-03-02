#include "radiosettings.h"
#include "ui_radiosettings.h"
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>


SoapySDR::KwargsList results;
RadioSettings::RadioSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RadioSettings)
{
    ui->setupUi(this);
    setWindowTitle("Radio Settings");

}

RadioSettings::~RadioSettings()
{
    delete ui;
}

void RadioSettings::Setup(SDR_Device_Manager *devs)
{
    m_devs = devs;
    ListRadios();
}

void RadioSettings::ListRadios()
{
    ui->lstradios->clear();
    //results = SoapySDR::Device::enumerate("driver=hackrf");
    SoapySDR::KwargsList results = m_devs->results;
    SoapySDR::Kwargs::iterator it;

    for( unsigned int i = 0; i < results.size(); ++i)
    {
        for( it = results[i].begin(); it != results[i].end(); ++it)
        {
            if(it->first == "device")
            {
                ui->lstradios->addItem(QString(it->second.c_str()));
            }
        }
    }
    if(results.size() > 0)
        ui->lstradios->setCurrentRow(0);
}

void RadioSettings::on_lstradios_currentRowChanged(int currentRow)
{
    QString txt = "";
    SoapySDR::Kwargs::iterator it;
    SoapySDR::KwargsList results = m_devs->results;
    for( it = results[currentRow].begin(); it != results[currentRow].end(); ++it)
    {
        txt += it->first.c_str();
        txt += " = ";
        txt += it->second.c_str();
        txt += "\r\n";
    }
    //now the details
    SDR_Device *dev = m_devs->m_devices[currentRow];


    //	2.1 antennas

    txt += "Rx antennas: \r\n";
    for(unsigned int i = 0; i < dev->lst_ant.size(); ++i)
        txt += QString(dev->lst_ant[i].c_str()) + "\r\n";


    //	2.2 gains

    txt += "Rx Gains: \r\n";
    for(unsigned int i = 0; i < dev->lst_gain.size(); ++i)
        txt += QString(dev->lst_gain[i].c_str()) + "\r\n";

    //	2.3. ranges(frequency ranges)
    txt += "Rx freq ranges: \r\n";
    for(unsigned int i = 0; i < dev->ranges.size(); ++i)
    {
        txt += QString::number(dev->ranges[i].minimum(),'f',2) + " -> " + QString::number(dev->ranges[i].maximum(),'f',2);
        txt += "\r\n";
    }
    ui->txtRadioDetails->setText(txt);
}
