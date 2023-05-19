#include "AlgorithmStrategyFactory.h"

Algorithm::AlgorithmStrategyFactory::AlgorithmStrategyFactory()
{
	m_hilbert = new Strategy::HilbertEnveloppe();
};

Algorithm::AlgorithmStrategyFactory::~AlgorithmStrategyFactory()
{
	EEGFormat::Utility::DeleteAndNullify(m_hilbert);
};

Algorithm::Strategy::IAlgorithmStrategy* Algorithm::AlgorithmStrategyFactory::GetAlgorithmStrategy(Strategy::AlgorithmType algo)
{
	switch (algo)
	{
	case Strategy::AlgorithmType::Hilbert:
		return m_hilbert;
		break;
	default:
		return nullptr;
		break;
	}
}