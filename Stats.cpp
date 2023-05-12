#include "Stats.h"

void InsermLibrary::Stats::pValuesWilcoxon(vec3<float> &pValue3D, vec3<int> &pSign3D, vec3<float> &bigdata, TriggerContainer* triggerContainer, int samplingFreq, ProvFile* myprovFile)
{
	std::vector<std::tuple<int, int, int>> CodeAndTrialsIndexes = triggerContainer->CodeAndTrialsIndexes();
    int ConditionCount = static_cast<int>(CodeAndTrialsIndexes.size());
    int ChannelCount = static_cast<int>(bigdata.size());
    for (int i = 0; i < ChannelCount; i++)
	{
		vec2<float> p_valueBig;
		vec2<int> p_signeBig;
		for (int j = 0; j < ConditionCount; j++)
		{
            int windowCount = GetNumberOfWindowForBloc(myprovFile, j);

            int lowTrial = std::get<1>(CodeAndTrialsIndexes[j]);
            int highTrial = std::get<2>(CodeAndTrialsIndexes[j]);
			int numberSubTrial = highTrial - lowTrial;
            if(numberSubTrial > 0)
            {
                vec1<double> baseLine = getBaselineBlocWilcoxon(i, lowTrial, numberSubTrial, samplingFreq, myprovFile->Blocs()[j].MainSubBloc(), bigdata);
                vec2<double> eegDataBig = getEegDataBlocWilcoxon(i, lowTrial, numberSubTrial, windowCount, samplingFreq, bigdata);

                vec1<float> p_value;
                for (int l = 0; l < static_cast<int>(eegDataBig.size()); l++)
                {
                    std::pair<double,double> pz = Framework::Calculations::Stats::wilcoxon_rank_sum(baseLine, eegDataBig[l]);
                    p_value.push_back(pz.first);
                }
                p_valueBig.push_back(p_value);
                p_signeBig.push_back(getEegSignBlocWilcoxon(baseLine, eegDataBig));
            }
            else
            {
                p_valueBig.push_back(std::vector<float>(windowCount, 0));
                p_signeBig.push_back(std::vector<int>(windowCount, 0));
            }
		}
		pSign3D.push_back(p_signeBig);
		pValue3D.push_back(p_valueBig);
	}
}

void InsermLibrary::Stats::pValuesKruskall(vec3<float> &pValue3D, vec3<int> &pSign3D, vec3<float> &bigdata, TriggerContainer* triggerContainer, int samplingFreq, ProvFile* myprovFile)
{
	// Get biggest window possible, for now we use the assumption that every bloc has the same window
	// TODO : deal with possible different windows
	int StartInSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().Start() * samplingFreq) / 1000;
	int EndinSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().End() * samplingFreq) / 1000;
	int* windowSam = new int[2]{ StartInSam, EndinSam };

    for (int i = 0; i < static_cast<int>(bigdata[0].size()); i++)
	{
		vec1<float> baseLineData = getBaselineKruskall(bigdata, triggerContainer, i, windowSam);
		vec2<float> eegDataBig = getEEGDataKruskall(bigdata, triggerContainer, i, windowSam);
		pValue3D.push_back(getPValuesKruskall(baseLineData, eegDataBig));
		pSign3D.push_back(getEegSignKruskall(baseLineData, eegDataBig));
	}
	delete[] windowSam;
}

InsermLibrary::vec1<InsermLibrary::PVALUECOORD> InsermLibrary::Stats::FDR(vec3<float> &pValues3D, vec3<int> &pSign3D, int &copyIndex, float pLimit)
{
	int V = pValues3D.size() * pValues3D[0].size() * pValues3D[0][0].size();
	float CV = log(V) + 0.5772;
	float slope = pLimit / (V * CV);

	vec1<PVALUECOORD> preFDRValues = loadPValues(pValues3D, pSign3D);

	std::sort(preFDRValues.begin(), preFDRValues.end(),
		[](PVALUECOORD firstValue, PVALUECOORD secondValue) {
		return (firstValue.pValue < secondValue.pValue);
	});

	for (int i = 0; i < V; i++)
	{
		if (preFDRValues[i].pValue >((double)slope * (i + 1)))
		{
			copyIndex = i;
			break;
		}
	}

    std::vector<PVALUECOORD> significantValue;
	for (int i = 0; i < copyIndex; i++)
	{
		significantValue.push_back(preFDRValues[i]);
	}

	//verifier si celui la est necessaire ??
	std::sort(significantValue.begin(), significantValue.end(),
		[](PVALUECOORD firstValue, PVALUECOORD secondValue) {
		return (firstValue.pValue < secondValue.pValue);
	});

	return significantValue;
}

InsermLibrary::vec1<InsermLibrary::PVALUECOORD> InsermLibrary::Stats::loadPValues(vec3<float> &pValues3D, vec3<int> &pSign3D)
{
    int count = 0;
	PVALUECOORD tempPValue;
    std::vector<PVALUECOORD> pValues;

    for (int i = 0; i < static_cast<int>(pValues3D.size()); i++)
	{
        for (int j = 0; j < static_cast<int>(pValues3D[i].size()); j++)
		{
            for (int k = 0; k < static_cast<int>(pValues3D[i][j].size()); k++)
			{
				tempPValue.elec = i;
				tempPValue.condit = j;
				tempPValue.window = k;
                tempPValue.vectorpos = count;
				tempPValue.pValue = pValues3D[i][j][k];
				tempPValue.weight = pSign3D[i][j][k];
				pValues.push_back(tempPValue);
                count++;
			}
		}
	}

	return pValues;
}

InsermLibrary::vec1<InsermLibrary::PVALUECOORD> InsermLibrary::Stats::loadPValues(vec3<float> &pValues3D, vec3<int> &pSign3D, float pLimit)
{
    int count = 0;
	PVALUECOORD tempPValue;
    std::vector<PVALUECOORD> pValues;

    for (int i = 0; i < static_cast<int>(pValues3D.size()); i++)
	{
        for (int j = 0; j < static_cast<int>(pValues3D[i].size()); j++)
		{
            for (int k = 0; k < static_cast<int>(pValues3D[i][j].size()); k++)
			{
				if (pValues3D[i][j][k] < pLimit)
				{
					tempPValue.elec = i;
					tempPValue.condit = j;
					tempPValue.window = k;
                    tempPValue.vectorpos = count;
					tempPValue.pValue = pValues3D[i][j][k];
					tempPValue.weight = pSign3D[i][j][k];

					pValues.push_back(tempPValue);
				}
                count++;
			}
		}
	}

	return pValues;
}

void InsermLibrary::Stats::exportStatsData(eegContainer *myEegContainer, ProvFile* myprovFile, vec1<PVALUECOORD> pValues, std::string outputFolder, bool isBar)
{
    std::ofstream fichierSt(outputFolder + "/statLoca.csv", std::ios::out);
	fichierSt << " " << ";";
	for (int i = 0; i < myEegContainer->BipoleCount(); i++)
	{
		fichierSt << myEegContainer->flatElectrodes[myEegContainer->Bipole(i).first] << ";";
	}
    fichierSt << std::endl;

    int blocCount = static_cast<int>(myprovFile->Blocs().size());
    for (int j = 0; j < blocCount; j++)
	{
		fichierSt << myprovFile->Blocs()[j].MainSubBloc().MainEvent().Name() << ";";
		for (int i = 0; i < myEegContainer->BipoleCount(); i++)
		{
            std::vector<int> indexes;
            for (int z = 0; z < static_cast<int>(pValues.size()); z++)
			{
				if (isBar == false)
				{
					if (pValues[z].elec == i && pValues[z].condit == j)
						indexes.push_back(z);
				}
				else
				{
					if (pValues[z].elec == i && pValues[z].window == j)
						indexes.push_back(z);
				}
			}

            int indexesCount = static_cast<int>(indexes.size());
            if (indexesCount > 0)
			{
				int val = 0;
                for (int z = 0; z < indexesCount; z++)
				{
					val += pValues[indexes[z]].weight;
				}
				fichierSt << val << ";";
			}
			else
			{
				fichierSt << "FALSE" << ";";
			}
		}
        fichierSt << std::endl;
	}
	fichierSt.close();
}

InsermLibrary::vec1<double> InsermLibrary::Stats::getBaselineBlocWilcoxon(int currentChanel, int lowTrial, int numberSubTrial, int samplingFreq, SubBloc dispBloc, vec3<float> &bigdata)
{
    vec1<double> baseLine;
	for (int k = 0; k < numberSubTrial; k++)
	{
        double sum = 0.0;
		int begBaseline = round((samplingFreq * (dispBloc.Baseline().Start() - dispBloc.MainWindow().Start())) / 1000);
		int endBaseline = round((samplingFreq * (dispBloc.Baseline().End() - dispBloc.MainWindow().Start())) / 1000);

		for (int l = 0; l < (endBaseline - begBaseline); l++)
		{
			sum += bigdata[currentChanel][lowTrial + k][begBaseline + l];
		}
		baseLine.push_back(sum / (endBaseline - begBaseline));
	}
	return baseLine;
}

int InsermLibrary::Stats::GetNumberOfWindowForBloc(ProvFile* myprovFile, int blocIndex)
{
    int numberWindows = 0;
    if (myprovFile->FilePath().find("INVERTED") != std::string::npos)
    {	//100 because 200ms window with overlap 50%
        numberWindows = myprovFile->Blocs()[blocIndex].MainSubBloc().MainWindow().Length() / 100;
    }
    else
    {
        numberWindows = myprovFile->Blocs()[blocIndex].MainSubBloc().MainWindow().End() / 100;
    }
    return numberWindows;
}

InsermLibrary::vec2<double> InsermLibrary::Stats::getEegDataBlocWilcoxon(int currentChanel, int lowTrial, int numberSubTrial, int numberWindows, int samplingFreq, vec3<float> &bigdata)
{
    vec2<double> eegDataBig;
	for (int k = 0; k < numberWindows - 1; k++)
	{
        int windowLengthMs = 100 * k;

        vec1<double> eegData;
		for (int l = 0; l < numberSubTrial; l++)
		{
            double sum = 0.0;
            int begWindow = round((samplingFreq * (0 + windowLengthMs)) / 1000);
            int endWindow = round((samplingFreq * (200 + windowLengthMs)) / 1000);

			for (int m = 0; m < (endWindow - begWindow); m++)
			{
				sum += bigdata[currentChanel][lowTrial + l][begWindow + m];
			}
			eegData.push_back(sum / (endWindow - begWindow));
		}
		eegDataBig.push_back(eegData);
	}

	return eegDataBig;
}

InsermLibrary::vec1<int> InsermLibrary::Stats::getEegSignBlocWilcoxon(vec1<double> &baseLine, vec2<double> &eegDataBig)
{
	vec1<int> valueSigne;
    for (int i = 0; i < static_cast<int>(eegDataBig.size()); i++)
	{
		double diff = 0;
		double meanBaseLine = 0, meanWindow = 0;
        for (int m = 0; m < static_cast<int>(eegDataBig[i].size()); m++)
		{
			meanBaseLine += baseLine[m];
			meanWindow += eegDataBig[i][m];
		}
		meanBaseLine /= eegDataBig[i].size();
		meanWindow /= eegDataBig[i].size();

		diff = meanWindow - meanBaseLine;
		if (diff < 0)
		{
			valueSigne.push_back(-1);
		}
		else if (diff == 0)
		{
			valueSigne.push_back(0);
		}
		else if (diff > 0)
		{
			valueSigne.push_back(1);
		}
	}
	return valueSigne;
}

InsermLibrary::vec1<float> InsermLibrary::Stats::getBaselineKruskall(vec3<float> &bigdata, TriggerContainer* triggerContainer, int currentChanel, int* windowSam)
{
	std::vector<std::tuple<int, int, int>> CodeAndTrialsIndexes = triggerContainer->CodeAndTrialsIndexes();

	int SubGroupCount = CodeAndTrialsIndexes.size();// triggerContainer->SubGroupStimTrials().size();
	//std::vector<int> SubGroupStimTrials = triggerContainer->SubGroupStimTrials();

	int lowTrial = std::get<1>(CodeAndTrialsIndexes[SubGroupCount - 1]);
	int highTrial = std::get<2>(CodeAndTrialsIndexes[SubGroupCount - 1]);
	int numberSubTrial = highTrial - lowTrial;

	vec1<float> baseLineData;
	for (int i= 0; i < numberSubTrial; i++)
	{
		int begBaseline = 0;
		int endBaseLine = windowSam[1] - windowSam[0];
		float sum = 0.0;
		for (int j = 0; j < (endBaseLine - begBaseline); j++)
		{
			sum += bigdata[lowTrial + i][currentChanel][begBaseline + j];
		}
		baseLineData.push_back(sum / (endBaseLine - begBaseline));
	}

	return baseLineData;
}

InsermLibrary::vec2<float> InsermLibrary::Stats::getEEGDataKruskall(vec3<float> &bigdata, TriggerContainer* triggerContainer, int currentChanel, int* windowSam)
{
	std::vector<std::tuple<int, int, int>> CodeAndTrialsIndexes = triggerContainer->CodeAndTrialsIndexes();

	int SubGroupCount = CodeAndTrialsIndexes.size();// triggerContainer->SubGroupStimTrials().size();
	//std::vector<int> SubGroupStimTrials = triggerContainer->SubGroupStimTrials();

	vec2<float> eegDataBig;
	for (int j = 0; j < SubGroupCount - 1; j++)
	{
		int lowTrial = std::get<1>(CodeAndTrialsIndexes[j]); //SubGroupStimTrials[j];
		int highTrial = std::get<2>(CodeAndTrialsIndexes[j]); //SubGroupStimTrials[j + 1];
		int numberSubTrial = highTrial - lowTrial;

		vec1<float> eegData;
		for (int k = 0; k < numberSubTrial; k++)
		{
			int begWindow = 0;
			int endWindow = windowSam[1] - windowSam[0];

			float sum = 0.0;
			for (int l = 0; l < (endWindow - begWindow); l++)
			{
				sum += bigdata[lowTrial + k][currentChanel][begWindow + l];
			}
			eegData.push_back(sum / (endWindow - begWindow));
		}
		eegDataBig.push_back(eegData);
	}
	return eegDataBig;
}

InsermLibrary::vec2<float> InsermLibrary::Stats::getPValuesKruskall(vec1<float> &baseLineData, vec2<float> &eegData)
{
	vec1<float> pValue;
	vec2<float> pValueBig;

	float *dataArray[2];
	int nbSamplePerGroup[2];
	double p = 0, H = 0;
	dataArray[0] = &baseLineData[0];
    for (int j = 0; j < static_cast<int>(eegData.size()); j++)
	{
		dataArray[1] = &eegData[j][0];
		nbSamplePerGroup[0] = eegData[j].size();
		nbSamplePerGroup[1] = eegData[j].size();

		Framework::Calculations::Stats::kruskal_wallis(dataArray, 2, nbSamplePerGroup, &H, &p, 1);
		pValue.push_back(p);
	}
	pValueBig.push_back(pValue);

	return pValueBig;
}

InsermLibrary::vec2<int> InsermLibrary::Stats::getEegSignKruskall(vec1<float> &baseLineData, vec2<float> &eegData)
{
	vec2<int> valueSignBig;
	vec1<int> valueSign;
    for (int j = 0; j < static_cast<int>(eegData.size()); j++)
	{
		double diff = 0;
		double meanBaseLine = 0, meanWindow = 0;
        for (int k = 0; k < static_cast<int>(eegData[j].size()); k++)
		{
			meanBaseLine += baseLineData[k];
			meanWindow += eegData[j][k];
		}
		meanBaseLine /= eegData[j].size();
		meanWindow /= eegData[j].size();

		diff = meanWindow - meanBaseLine;
		if (diff < 0)
		{
			valueSign.push_back(-1);
		}
		else if (diff == 0)
		{
			valueSign.push_back(0);
		}
		else if (diff > 0)
		{
			valueSign.push_back(1);
		}
	}
	valueSignBig.push_back(valueSign);
	return valueSignBig;
}
