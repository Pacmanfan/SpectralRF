#ifndef SDR_DEVICE_MANAGER_H
#define SDR_DEVICE_MANAGER_H

#include <QList>
#include <sdr_device.h>

class SDR_Device_Manager
{
public:
    SDR_Device_Manager();
    QList<SDR_Device *> m_devices;
    SoapySDR::KwargsList results;
    void DetectAll();
    void CloseAll();
};

#endif // SDR_DEVICE_MANAGER_H
