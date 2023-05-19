#ifndef ALGORITHM_CALCULATOR_H
#define ALGORITHM_CALCULATOR_H

#include <iostream>
#include <vector>
#include "AlgorithmStrategyFactory.h"
#include "eegContainer.h"

namespace Algorithm
{
	class AlgorithmCalculator
	{
	public:
		static void ExecuteAlgorithm(Algorithm::Strategy::AlgorithmType algo, InsermLibrary::eegContainer* EegContainer, const std::vector<int>& FrequencyBand);

	private:
		static AlgorithmStrategyFactory m_factory;
	};
}
#endif