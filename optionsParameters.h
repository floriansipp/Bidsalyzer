#ifndef OPTIONSPARAMETERS_H
#define OPTIONSPARAMETERS_H

#include <iostream>
#include <vector>

#include <QGroupBox>
#include <QCheckBox>
#include <QFrame>

namespace InsermLibrary
{
	struct statOption
	{
		bool wilcoxon = true;
		bool FDRwilcoxon = true;
        float pWilcoxon = 0.05;
		bool kruskall = true;
		bool FDRkruskall = true;
        float pKruskall = 0.05;
        bool BlocWiseStatistics = false;
	};

	struct picOption
	{
		QSize sizeTrialmap;
		QSize interpolationtrialmap;
		QSize sizePlotmap;
	};

	struct PVALUECOORD
	{
		int elec = -69;
		int condit = -69;
		int window = -69;
		int vectorpos = -69;
		float pValue = -69;
		int weight = 0;
	};

    struct PVALUECOORD_KW
    {
        int elec = -69;
        int condit1 = -69;
        int condit2 = -69;
        int window = -69;
        int vectorpos = -69;
        float pValue = -69;
        int weight = 0;
    };
}
#endif
