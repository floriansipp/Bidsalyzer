#ifndef ALGORITHM_STRATEGY_FACTORY_H
#define ALGORITHM_STRATEGY_FACTORY_H

#include <iostream>
#include "IAlgorithmStrategy.h"
#include "HilbertEnveloppe.h"

namespace Algorithm
{
	class AlgorithmStrategyFactory
	{
	public:
		AlgorithmStrategyFactory();
		~AlgorithmStrategyFactory();
		Strategy::IAlgorithmStrategy* GetAlgorithmStrategy(Strategy::AlgorithmType algo);

	private:
		Strategy::HilbertEnveloppe* m_hilbert = nullptr;
	};
}
#endif
