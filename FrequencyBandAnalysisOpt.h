#ifndef FREQUENCYBANDANALYSISOPT_H
#define FREQUENCYBANDANALYSISOPT_H

#include <iostream>
#include <vector>

#include "../../EEGFormat/EEGFormat/IFile.h"
#include "IAlgorithmStrategy.h"
#include "Data/FrequencyBand.h"

namespace InsermLibrary
{
	struct FrequencyAnalysis
	{
		bool eeg2env2;
		Algorithm::Strategy::AlgorithmType calculationType = Algorithm::Strategy::AlgorithmType::Hilbert;
		EEGFormat::FileType outputType = EEGFormat::FileType::Elan;
	};

	struct FrequencyBandAnalysisOpt
	{
		FrequencyAnalysis analysisParameters;
		bool env2plot;
		bool trialmat;
		bool correMaps;
        bool statFiles;
        int smoothingIDToUse = 0;
		FrequencyBand Band;
	};
}

#endif
