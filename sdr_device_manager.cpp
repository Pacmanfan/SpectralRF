#include "sdr_device_manager.h"

SDR_Device_Manager::SDR_Device_Manager()
{
}

void SDR_Device_Manager::DetectAll()
{
    CloseAll();
    // 0. enumerate devices (list all devices' information)
    results = SoapySDR::Device::enumerate();
    for( unsigned int i = 0; i < results.size(); ++i)
    {
        SoapySDR::Kwargs args = results[i];
        SDR_Device *dev = new SDR_Device();
        bool ret = dev->InitSDR(args);
        if(ret == true)
            m_devices.append(dev);
    }
}

void SDR_Device_Manager::CloseAll()
{
    for(int c= 0; c < m_devices.size() ; c ++)
    {
        SDR_Device *dev = m_devices[c];
        delete dev; // will close it down
    }
    m_devices.clear();
    results.clear();
}
