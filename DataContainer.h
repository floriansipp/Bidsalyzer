#ifndef _DATACONTAINER_H
#define _DATACONTAINER_H

#include "./../../EEGFormat/EEGFormat/IFile.h"
#include "./../../Framework/Framework/FirBandPass.h"

namespace InsermLibrary
{
	struct DataContainer
	{
	public:
		DataContainer(int samplingFrequency, int downsampledFrequency, int nbSample, std::vector<int> frequencyBand);
		~DataContainer();

		inline int NbSample()
		{
			return m_nbSample;
		}
		inline int NbSampleDownsampled()
		{
			return m_nbSampleDownsampled;
		}
		inline int NbFrequencySlices()
		{
			return m_frequencyLength;
		}

		std::vector<std::vector<float>> bipData;
		std::vector<std::vector<float>> hilData;
		std::vector<std::vector<Framework::Filtering::Linear::FirBandPass*>> Filters;
		std::vector<std::vector<float>>downData;
		std::vector<std::vector<float>>meanData;
		std::vector<std::vector<std::vector<float>>> convoData;

	private:
		int m_nbSample;
		int m_nbSampleDownsampled;
		int m_frequencyLength;
		int m_samplingFrequency;
		int m_downsampledFrequency;
	};
}

#endif
