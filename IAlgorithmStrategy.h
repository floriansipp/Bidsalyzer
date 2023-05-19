#ifndef IALGORITHMSTRATEGY_H
#define IALGORITHMSTRATEGY_H

#include <iostream>
#include "eegContainer.h"

namespace Algorithm
{
	namespace Strategy
	{
		enum class AlgorithmType { Hilbert, Unknown };

		inline AlgorithmType GetFileTypeFromString(const std::string& str)
		{
			if (str.compare("Hilbert Transform") == 0)
			{
				return AlgorithmType::Hilbert;
			}
			else
			{
				return AlgorithmType::Unknown;
			}
		}

		class IAlgorithmStrategy
		{
		public:
            virtual void Process(InsermLibrary::eegContainer* EegContainer, std::vector<int> FrequencyBand) = 0;
		};
	}
}


#endif
