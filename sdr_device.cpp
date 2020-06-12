#include "sdr_device.h"

SDR_Device::SDR_Device()
{
    sdr = nullptr; //interface to the sdr device
    rx_stream = nullptr;// the RX stream
}

SDR_Device::~SDR_Device()
{
    CloseSDR();
}

bool SDR_Device::InitSDR(SoapySDR::Kwargs &args)
{
    //	1.2 make device
    sdr = SoapySDR::Device::make(args);
    if( sdr == nullptr )
    {
        return false;
    }

    lst_ant = sdr->listAntennas( SOAPY_SDR_RX, 0);
    lst_gain = sdr->listGains( SOAPY_SDR_RX, 0);
    ranges = sdr->getFrequencyRange( SOAPY_SDR_RX, 0);
    for(unsigned int i = 0; i < ranges.size(); ++i)
    {
        m_freq_limit_low = ranges[i].minimum();
        m_freq_limit_high = ranges[i].maximum();
    }
    SoapySDR::RangeList rates = sdr->getSampleRateRange(SOAPY_SDR_RX,0);
    for(unsigned int i = 0; i < rates.size(); ++i)
    {
        if(i == 0)
        {
            m_rate_limit_low = rates[i].minimum();
            m_rate_limit_high = rates[i].maximum();
        }
        else
        {
            if(rates[i].minimum() < m_rate_limit_low) m_rate_limit_low = rates[i].minimum();
            if(rates[i].maximum() > m_rate_limit_high) m_rate_limit_high = rates[i].maximum();
        }
    }

    sdr->setSampleRate( SOAPY_SDR_RX, 0, m_rate_limit_high);// set to max bw
    m_BW_Hz = m_rate_limit_high;

    sdr->setFrequency( SOAPY_SDR_RX, 0, 433e6);
    // 4. setup a stream (complex floats)
    rx_stream = sdr->setupStream( SOAPY_SDR_RX, SOAPY_SDR_CF32);
    if( rx_stream == nullptr)
    {
        SoapySDR::Device::unmake( sdr );
        sdr = nullptr;
        return false;
    }
    return true;
}

void SDR_Device::CloseSDR()
{
    // 7. shutdown the stream
    if(rx_stream != nullptr)
    {
        sdr->deactivateStream( rx_stream, 0, 0);	//stop streaming
        sdr->closeStream( rx_stream );
    }
    // 8. cleanup device handle
    if(sdr !=nullptr)
    {
        SoapySDR::Device::unmake( sdr );
    }
}
