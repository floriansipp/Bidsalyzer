#include "FrequencyBand.h"

InsermLibrary::FrequencyBand::FrequencyBand()
{
    m_name = "New Frequency Band";
    UpdateFrequencies(10, 50, 10);
}

InsermLibrary::FrequencyBand::FrequencyBand(const std::string& bandName, int fMin, int fMax, int step)
{
	m_name = bandName;

	for (int i = 0; i <= ((fMax - fMin) / step); i++)
	{
		m_bins.push_back(fMin + (i * step));
	}
}

void InsermLibrary::FrequencyBand::CheckShannonCompliance(int samplingFrequency)
{
	if (!(samplingFrequency > (2 * FMax())))
	{
		int fMin = FMin();
		int step = Step();
		int fMax = ((samplingFrequency / 2) / step) * step;

		m_bins.clear();
		for (int i = 0; i <= ((fMax - fMin) / step); i++)
			m_bins.push_back(fMin + (i * step));
	}
}

void InsermLibrary::FrequencyBand::UpdateFrequencies(int fMin, int fMax, int step)
{
    m_bins.clear();
    for (int i = 0; i <= ((fMax - fMin) / step); i++)
    {
        m_bins.push_back(fMin + (i * step));
    }
}
