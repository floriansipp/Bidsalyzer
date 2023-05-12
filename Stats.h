#ifndef _STATS_H
#define _STATS_H

#include <iostream>
#include <fstream>		
#include "ProvFile.h"
#include "TriggerContainer.h"
#include "eegContainer.h"

#include "./../../Framework/Framework/Wilcox.h"
#include "./../../Framework/Framework/Kruskall.h"
#include "Utility.h"
#include "optionsParameters.h"

namespace InsermLibrary
{
	class Stats
	{
	public:
		static void pValuesWilcoxon(vec3<float> &pValue3D, vec3<int> &pSign3D, vec3<float> &bigdata, TriggerContainer* triggerContainer,
									int samplingFreq, ProvFile* myprovFile);
		static void pValuesKruskall(vec3<float> &pValue3D, vec3<int> &pSign3D, vec3<float> &bigdata, TriggerContainer* triggerContainer,
									int samplingFreq, ProvFile* myprovFile);
		static vec1<PVALUECOORD> FDR(vec3<float> &pValues3D, vec3<int> &pSign3D, int &copyIndex, float pLimit);
		static vec1<PVALUECOORD> loadPValues(vec3<float> &pValues3D, vec3<int> &pSign3D);
		static vec1<PVALUECOORD> loadPValues(vec3<float> &pValues3D, vec3<int> &pSign3D, float pLimit);
		static void exportStatsData(eegContainer *myEegContainer, ProvFile* myprovFile, vec1<PVALUECOORD> pValues,
                                    std::string outputFolder, bool isBar);
	private:
        static vec1<double> getBaselineBlocWilcoxon(int currentChanel, int lowTrial, int numberSubTrial, int samplingFreq, SubBloc dispBloc, vec3<float> &bigdata);
        static int GetNumberOfWindowForBloc(ProvFile* myprovFile, int blocIndex);
        static vec2<double> getEegDataBlocWilcoxon(int currentChanel, int lowTrial, int numberSubTrial, int numberWindows, int samplingFreq, vec3<float> &bigdata);
        static vec1<int> getEegSignBlocWilcoxon(vec1<double> &baseLine, vec2<double> &eegDataBig);
		//==
		static vec1<float> getBaselineKruskall(vec3<float> &bigdata, TriggerContainer* triggerContainer, int currentChanel, int* windowSam);
		static vec2<float> getEEGDataKruskall(vec3<float> &bigdata, TriggerContainer* triggerContainer, int currentChanel, int* windowSam);
		static vec2<float> getPValuesKruskall(vec1<float> &baseLineData, vec2<float> &eegData);
		static vec2<int> getEegSignKruskall(vec1<float> &baseLineData, vec2<float> &eegData);
	};
}

#endif
