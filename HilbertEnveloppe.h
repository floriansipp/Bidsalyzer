#ifndef HILBERTENVELOPPE_H
#define HILBERTENVELOPPE_H

#include <iostream>
#include "IAlgorithmStrategy.h"
#include "eegContainer.h"
#include "./../../Framework/Framework/Convolution.h"

namespace Algorithm
{
	namespace Strategy
	{
		class HilbertEnveloppe : public IAlgorithmStrategy
		{
		public:
            HilbertEnveloppe() { }
            ~HilbertEnveloppe() { }
			virtual void Process(InsermLibrary::eegContainer* Container, std::vector<int> FrequencyBand);

		private:
			void InitOutputDataStructure(InsermLibrary::eegContainer* EegContainer);
            void SetInputDataFromBipoles(std::vector<float> & inputData, std::pair<int, int> bipoles, int sampleCount, InsermLibrary::eegContainer* EegContainer);
            void CalculateSmoothingCoefficients(int DownsampledFrequency);
			void HilbertDownSampSumData(InsermLibrary::DataContainer* DataContainer, int threadId, int freqId);
			void MeanConvolveData(InsermLibrary::DataContainer *DataContainer, int threadId);

		private:
			float m_smoothingSample[6];
			float m_smoothingMilliSec[6] = { 0, 250, 500, 1000, 2500, 5000 };
			std::mutex m_mtx;
		};
	}
}

#endif
