#include "DataContainer.h"

using Framework::Filtering::Linear::FirBandPass;

InsermLibrary::DataContainer::DataContainer(int samplingFrequency, int downsampledFrequency, int nbSample, std::vector<int> frequencyBand)
{
	m_samplingFrequency = samplingFrequency;
	m_downsampledFrequency = downsampledFrequency;
	m_nbSample = nbSample;
	m_nbSampleDownsampled = m_nbSample / (m_samplingFrequency / m_downsampledFrequency);
	m_frequencyLength = (int)frequencyBand.size();

	bipData.resize(5, std::vector<float>(m_nbSample));
	hilData.resize(5, std::vector<float>(m_nbSample));

	downData.resize(5, std::vector<float>(m_nbSampleDownsampled));
	meanData.resize(5, std::vector<float>(m_nbSampleDownsampled));
	convoData.resize(6, std::vector<std::vector<float>>(5, std::vector<float>(m_nbSampleDownsampled)));

	//TODO : It is more efficient for memory usage to declara the 5 fftForward and 5 fftBackward 
	//objects outside of the firBandPass for memory usage. 
	//See if it's not possible to improve the paralellisation of the calcul (omp for vs threads) 
	Filters.resize(5, std::vector<FirBandPass*>());
	int nbFrequencySlice = frequencyBand.size() - 1;
	for (int j = 0; j < 5; j++)
	{
		for (int i = 0; i < nbFrequencySlice; i++)
			Filters[j].push_back(new FirBandPass(frequencyBand[i], frequencyBand[i + 1], m_samplingFrequency, m_nbSample));
	}
}

InsermLibrary::DataContainer::~DataContainer()
{
	for (int i = 0; i < Filters.size(); i++)
	{
		for (int j = 0; j < Filters[i].size(); j++)
		{
			EEGFormat::Utility::DeleteAndNullify(Filters[i][j]);
		}
	}
	Filters.clear();
}