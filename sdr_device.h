#ifndef SDR_DEVICE_H
#define SDR_DEVICE_H

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>

#include <string>	// std::string
#include <vector>	// std::vector<...>
#include <map>		// std::map< ... , ... >

class SDR_Device
{
public:
    SDR_Device();
    ~SDR_Device();

    SoapySDR::Device *sdr; //interface to the sdr device
    SoapySDR::RangeList ranges; // the min /max ranges of the freq this radio can perform
    SoapySDR::Stream *rx_stream;// the RX stream
    std::vector< std::string > lst_ant; // list of antennae
    std::vector< std::string > lst_gain; // list of gains
    bool InitSDR(SoapySDR::Kwargs &args);
    void CloseSDR();
    double m_freq_limit_low, m_freq_limit_high; // the min/max freqs this radio is capable of.
    double m_rate_limit_low, m_rate_limit_high; // upper and lower ranges of the sample rate that the SDR is capable of.
    double m_BW; // the bandwidth / sps rate the radio is currently set at (usually max)
};

#endif // SDR_DEVICE_H
