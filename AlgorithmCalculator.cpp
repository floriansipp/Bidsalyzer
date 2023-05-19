#include "AlgorithmCalculator.h"

Algorithm::AlgorithmStrategyFactory Algorithm::AlgorithmCalculator::m_factory;

void Algorithm::AlgorithmCalculator::ExecuteAlgorithm(Algorithm::Strategy::AlgorithmType algo, InsermLibrary::eegContainer* EegContainer, const std::vector<int>& FrequencyBand)
{
	Strategy::IAlgorithmStrategy* AlgorithmStrategy = m_factory.GetAlgorithmStrategy(algo);
	if (AlgorithmStrategy != nullptr)
	{
		AlgorithmStrategy->Process(EegContainer, FrequencyBand);
	}
}

