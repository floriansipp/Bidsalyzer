#include "StatisticalFilesProcessor.h"
#include <QDebug>

void InsermLibrary::StatisticalFilesProcessor::Process(TriggerContainer* triggerContainer, eegContainer* myeegContainer, int smoothingID, ProvFile* myprovFile, std::string freqFolder, statOption* statOption)
{
    int StartInSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().Start() * myeegContainer->DownsampledFrequency()) / 1000;
    int EndinSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().End() * myeegContainer->DownsampledFrequency()) / 1000;
    int* windowSam = new int[2]{ StartInSam, EndinSam };

    //== get Bloc of eeg data we want to display center around events
    vec3<float> bigData;
    bigData.resize(myeegContainer->BipoleCount(), vec2<float>(triggerContainer->ProcessedTriggerCount(), vec1<float>(windowSam[1] - windowSam[0])));
    myeegContainer->GetFrequencyBlocData(bigData, smoothingID, triggerContainer->ProcessedTriggers(), windowSam);

    std::vector<std::vector<std::vector<double>>> Stat_Z_CCS, Stat_P_CCS;
    for(int i = 0;i < bigData.size(); i++)
    {
        std::vector<std::vector<double>> Stat_Z_CS, Stat_P_CS;
        std::vector<std::tuple<int, int, int>> CodeAndTrialsIndexes = triggerContainer->CodeAndTrialsIndexes();
        int blocCount = static_cast<int>(myprovFile->Blocs().size());
        for(int j = 0; j < blocCount; j++)
        {
            SubBloc subBloc = myprovFile->Blocs()[j].MainSubBloc();
            int baselineBegin = (((float)myeegContainer->DownsampledFrequency() * subBloc.Baseline().Start()) / 1000) - windowSam[0];
            int baselineEnd = (((float)myeegContainer->DownsampledFrequency() * subBloc.Baseline().End()) / 1000)  - windowSam[0];
            int windowBegin = (((float)myeegContainer->DownsampledFrequency() * subBloc.MainWindow().Start()) / 1000)  - windowSam[0];
            int windowEnd = (((float)myeegContainer->DownsampledFrequency() * subBloc.MainWindow().End())/1000)  - windowSam[0];

            vec1<double> baselineData = vec1<double>();
            vec2<double> conditionData = vec2<double>();
            std::vector<double> v_stat_Z, v_stat_P;
            int code = subBloc.MainEvent().Codes()[0];
            auto it = std::find_if(CodeAndTrialsIndexes.begin(), CodeAndTrialsIndexes.end(), [&](const std::tuple<int, int, int>& c) { return std::get<0>(c) == code; });
            if (it != CodeAndTrialsIndexes.end())
            {
                int codeIndex = std::distance(CodeAndTrialsIndexes.begin(), it);
                int beg = std::get<1>(CodeAndTrialsIndexes[codeIndex]);
                int end = std::get<2>(CodeAndTrialsIndexes[codeIndex]);
                if(end - beg > 0)
                {
                    //Copy needed data , all trials for one condition and the associated samples
                    baselineData = vec1<double>();
                    conditionData = vec2<double>(end-beg, vec1<double>());
                    for(int k = 0; k < conditionData.size(); k++)
                    {
                        int sum = std::accumulate(bigData[i][beg+k].begin() + baselineBegin, bigData[i][beg+k].begin() + baselineEnd, 0);
                        double bl = static_cast<double>(sum) / (baselineEnd - baselineBegin);
                        baselineData.push_back(bl); //baseline mean for this trial

                        //copy relevant data
                        std::vector<float>::iterator begIter = bigData[i][beg + k].begin() + windowBegin;
                        std::vector<float>::iterator endIter = bigData[i][beg + k].begin() + windowEnd;
                        conditionData[k] = vec1<double>(begIter, endIter);
                    }

                    //loop over timebins
                    int timeBinsCount = static_cast<int>(conditionData[0].size());
                    for(int k = 0; k < timeBinsCount; k++)
                    {
                        std::vector<double> dataToCompare;
                        for(int l = 0; l < conditionData.size(); l++)
                        {
                            dataToCompare.push_back(conditionData[l][k]);
                        }

                        std::pair<double, double> pz = Framework::Calculations::Stats::wilcoxon_rank_sum(dataToCompare, baselineData);
                        v_stat_P.push_back(pz.first);
                        v_stat_Z.push_back(pz.second);
                    }
                }
            }

            Stat_P_CS.push_back(v_stat_P);
            Stat_Z_CS.push_back(v_stat_Z);
        }

        Stat_P_CCS.push_back(Stat_P_CS);
        Stat_Z_CCS.push_back(Stat_Z_CS);
    }

    //At this point we might have some empty vectors if there is a condition with no trigger in the experiment
    //so we cheat and resize those vectors and put fake values in it
    for(int i = 0; i < Stat_P_CCS.size(); i++)
    {
        int nbOfSample = 0;
        for(int j = 0; j < Stat_P_CCS[i].size(); j++)
        {
            if(Stat_P_CCS[i][j].size() != nbOfSample && Stat_P_CCS[i][j].size() > 0)
            {
                nbOfSample = Stat_P_CCS[i][j].size();
            }
        }

        for(int j = 0; j < Stat_P_CCS[i].size(); j++)
        {
            if(Stat_P_CCS[i][j].size() == 0)
            {
                Stat_P_CCS[i][j].resize(nbOfSample, 0);
                Stat_Z_CCS[i][j].resize(nbOfSample, 0);
            }
        }
    }

    std::vector<PVALUECOORD> significantValue;
    if (statOption->FDRwilcoxon)
    {
        int V = Stat_P_CCS.size() * Stat_P_CCS[0].size() * Stat_P_CCS[0][0].size();
        float CV = log(V) + 0.5772;
        float slope = statOption->pWilcoxon / (V * CV);

        std::vector<PVALUECOORD> preFDRValues = loadPValues(Stat_P_CCS);

        std::sort(preFDRValues.begin(), preFDRValues.end(),
            [](PVALUECOORD firstValue, PVALUECOORD secondValue) {
            return (firstValue.pValue < secondValue.pValue);
        });

        int copyIndex = 0;
        for (int i = 0; i < V; i++)
        {
            if (preFDRValues[i].pValue > ((double)slope * (i + 1)))
            {
                copyIndex = i;
                break;
            }
        }

        for (int i = 0; i < copyIndex; i++)
        {
            significantValue.push_back(preFDRValues[i]);
        }

        //verifier si celui la est necessaire ??
        std::sort(significantValue.begin(), significantValue.end(),
            [](PVALUECOORD firstValue, PVALUECOORD secondValue) {
            return (firstValue.pValue < secondValue.pValue);
        });
    }
    else
    {
        significantValue = loadPValues(Stat_P_CCS, statOption->pWilcoxon);
    }

    //Kruskall
    int windowSize = 0;
    int countFDR = 0;
    std::vector<std::vector<std::vector<std::vector<double>>>> v_stat_K4, v_stat_P4;
    for (int i = 0; i < bigData.size(); i++)
    {
        std::vector<std::vector<std::vector<double>>> v_stat_K3, v_stat_P3;
        std::vector<std::tuple<int, int, int>> CodeAndTrialsIndexes = triggerContainer->CodeAndTrialsIndexes();
        int blocCount = static_cast<int>(myprovFile->Blocs().size());
        for (int j = 0; j < blocCount - 1; j++)
        {
            SubBloc subBloc = myprovFile->Blocs()[j].MainSubBloc();
            int firstConditionWindowBegin = (((float)myeegContainer->DownsampledFrequency() * subBloc.MainWindow().Start()) / 1000) - windowSam[0];
            int firstConditionWindowEnd = (((float)myeegContainer->DownsampledFrequency() * subBloc.MainWindow().End()) / 1000)  - windowSam[0];

            windowSize = firstConditionWindowEnd - firstConditionWindowBegin;

            //copy relevant data
            std::vector<std::vector<double>> firstConditionData;
            int code = subBloc.MainEvent().Codes()[0];
            auto it = std::find_if(CodeAndTrialsIndexes.begin(), CodeAndTrialsIndexes.end(), [&](const std::tuple<int, int, int>& c) { return std::get<0>(c) == code; });
            if (it != CodeAndTrialsIndexes.end())
            {
                int codeIndex = std::distance(CodeAndTrialsIndexes.begin(), it);
                int firstConditionBeg = std::get<1>(CodeAndTrialsIndexes[codeIndex]);
                int firstConditionEnd = std::get<2>(CodeAndTrialsIndexes[codeIndex]);

                for (int k = 0; k < (firstConditionEnd - firstConditionBeg); k++)
                {
                    std::vector<float>::iterator begIter = bigData[i][firstConditionBeg + k].begin() + firstConditionWindowBegin;
                    std::vector<float>::iterator endIter = bigData[i][firstConditionBeg + k].begin() + firstConditionWindowEnd;
                    std::vector<double> data = vec1<double>(begIter, endIter);

                    if(statOption->BlocWiseStatistics)
                    {
                        firstConditionData.push_back(std::vector<double>{ std::reduce(data.begin(), data.end()) / data.size() });
                    }
                    else
                    {
                        firstConditionData.push_back(data);
                    }
                }
            }

            std::vector<std::vector<double>> v_stat_K2, v_stat_P2;
            for (int k = j + 1; k < blocCount; k++)
            {
                SubBloc secondSubBloc = myprovFile->Blocs()[k].MainSubBloc();
                int secondConditionWindowBegin = (((float)myeegContainer->DownsampledFrequency() * secondSubBloc.MainWindow().Start()) / 1000) - windowSam[0];
                int secondConditionWindowEnd = (((float)myeegContainer->DownsampledFrequency() * secondSubBloc.MainWindow().End()) / 1000) - windowSam[0];

                //qDebug() << firstConditionWindowEnd - firstConditionWindowBegin;

                std::vector<std::vector<double>> secondConditionData;
                std::vector<double> v_stat_K, v_stat_P;
                int secondCode = secondSubBloc.MainEvent().Codes()[0];

                auto secondIt = std::find_if(CodeAndTrialsIndexes.begin(), CodeAndTrialsIndexes.end(), [&](const std::tuple<int, int, int>& c) { return std::get<0>(c) == secondCode; });
                if (it != CodeAndTrialsIndexes.end() && secondIt != CodeAndTrialsIndexes.end())
                {
                    int secondCodeIndex = std::distance(CodeAndTrialsIndexes.begin(), secondIt);
                    int secondConditionBeg = std::get<1>(CodeAndTrialsIndexes[secondCodeIndex]);
                    int secondConditionEnd = std::get<2>(CodeAndTrialsIndexes[secondCodeIndex]);

                    //copy relevant data
                    for (int l = 0; l < (secondConditionEnd - secondConditionBeg); l++)
                    {
                        std::vector<float>::iterator begIter = bigData[i][secondConditionBeg + l].begin() + secondConditionWindowBegin;
                        std::vector<float>::iterator endIter = bigData[i][secondConditionBeg + l].begin() + secondConditionWindowEnd;
                        std::vector<double> data = vec1<double>(begIter, endIter);

                        if(statOption->BlocWiseStatistics)
                        {
                            secondConditionData.push_back(std::vector<double>{ std::reduce(data.begin(), data.end()) / data.size() });
                        }
                        else
                        {
                            secondConditionData.push_back(data);
                        }
                    }

                    //loop over timebins
                    int timeBinsCount = static_cast<int>(firstConditionData[0].size());
                    for (int l = 0; l < timeBinsCount; l++)
                    {
                        double firstMean = 0;
                        std::vector<float> firstDataToCompare;
                        for (int m = 0; m < firstConditionData.size(); m++)
                        {
                            firstMean += firstConditionData[m][l];
                            firstDataToCompare.push_back(firstConditionData[m][l]);
                        }
                        firstMean /= firstConditionData.size();

                        double secondMean = 0;
                        std::vector<float> secondDataToCompare;
                        for (int m = 0; m < secondConditionData.size(); m++)
                        {
                            secondMean += secondConditionData[m][l];
                            secondDataToCompare.push_back(secondConditionData[m][l]);
                        }
                        secondMean /= secondConditionData.size();

                        //TODO : first pass doesn't send back a p value (=0) , why ?
                        float* dataArray[2];
                        int nbSamplePerGroup[2];
                        double p = 0, H = 0;
                        dataArray[0] = firstDataToCompare.data();
                        dataArray[1] = secondDataToCompare.data();
                        nbSamplePerGroup[0] = firstDataToCompare.size();
                        nbSamplePerGroup[1] = secondDataToCompare.size();
                        Framework::Calculations::Stats::kruskal_wallis(dataArray, 2, nbSamplePerGroup, &H, &p, 1);

                        int sign_FirstMinusSecond = (firstMean - secondMean) < 0 ? -1 : 1;
                        v_stat_K.push_back(sign_FirstMinusSecond * ((p < statOption->pWilcoxon) ? 1 : 0 ));
                        v_stat_P.push_back(p);
                        countFDR++;
                    }
                }
                v_stat_K2.push_back(v_stat_K);
                v_stat_P2.push_back(v_stat_P);
            }
            v_stat_K3.push_back(v_stat_K2);
            v_stat_P3.push_back(v_stat_P2);
        }
        v_stat_K4.push_back(v_stat_K3);
        v_stat_P4.push_back(v_stat_P3);
    }

    //Fill the arrays with the same value in case of average to be viewable under hibop
    //otherwise with 0 when the condition is not present
    if(statOption->BlocWiseStatistics)
    {
        for(int i = 0; i < v_stat_K4.size(); i++)
        {
            for(int j = 0; j < v_stat_K4[i].size(); j++)
            {
                for(int k = 0; k < v_stat_K4[i][j].size(); k++)
                {
                    if(v_stat_P4[i][j][k].size() == 1)
                    {
                        //qDebug() << v_stat_P4[i][j][k].size();
                        v_stat_K4[i][j][k].resize(2 * windowSize, v_stat_K4[i][j][k][0]);
                        v_stat_P4[i][j][k].resize(2 * windowSize, v_stat_P4[i][j][k][0]);
                    }
                }
            }
        }
    }
    else
    {
        for(int i = 0; i < v_stat_K4.size(); i++)
        {
            int nbOfSample = 0;
            for(int j = 0; j < v_stat_K4[i].size(); j++)
            {
                for(int k = 0; k < v_stat_K4[i][j].size(); k++)
                {
                    if(v_stat_K4[i][j][k].size() != nbOfSample && v_stat_K4[i][j][k].size() > 0)
                    {
                        nbOfSample = v_stat_K4[i][j][k].size();
                    }
                }
            }

            for(int j = 0; j < v_stat_K4[i].size(); j++)
            {
                for(int k = 0; k < v_stat_K4[i][j].size(); k++)
                {
                    if(v_stat_P4[i][j][k].size() == 0)
                    {
                        v_stat_K4[i][j][k].resize(nbOfSample, 0);
                        v_stat_P4[i][j][k].resize(nbOfSample, 0);
                    }
                }
            }
        }
    }

    std::vector<PVALUECOORD_KW> significantValue2;
    if (statOption->FDRkruskall)
    {
        int V = v_stat_P4.size() * v_stat_P4[0].size() * v_stat_P4[0][0].size() * v_stat_P4[0][0][0].size();
        float CV = log(countFDR) + 0.5772;
        float slope = statOption->pKruskall / (countFDR * CV);

        std::vector<PVALUECOORD_KW> preFDRValues = loadPValues_KW(v_stat_P4);

        std::sort(preFDRValues.begin(), preFDRValues.end(),
            [](PVALUECOORD_KW firstValue, PVALUECOORD_KW secondValue) {
            return (firstValue.pValue < secondValue.pValue);
        });

        int copyIndex = 0;
        for (int i = 0; i < countFDR; i++)
        {
            if (preFDRValues[i].pValue > ((double)slope * (i + 1)))
            {
                copyIndex = i;
                break;
            }
        }

        for (int i = 0; i < copyIndex; i++)
        {
            significantValue2.push_back(preFDRValues[i]);
        }

        //verifier si celui la est necessaire ??
        std::sort(significantValue2.begin(), significantValue2.end(),
            [](PVALUECOORD_KW firstValue, PVALUECOORD_KW secondValue) {
            return (firstValue.pValue < secondValue.pValue);
        });
    }
    else
    {
        significantValue2 = loadPValues_KW(v_stat_P4, statOption->pKruskall);
    }

    std::vector<std::pair<int,int>> codesPairs;
    int blocCount = static_cast<int>(myprovFile->Blocs().size());
    for (int j = 0; j < blocCount - 1; j++)
    {
        std::vector<int> firstCodes = myprovFile->Blocs()[j].MainSubBloc().MainEvent().Codes();
        for (int k = j + 1; k < blocCount; k++)
        {
            std::vector<int> secondCodes = myprovFile->Blocs()[k].MainSubBloc().MainEvent().Codes();
            codesPairs.push_back(std::make_pair(firstCodes[0], secondCodes[0]));
        }
    }

    //at this point everything is processed, now we need to export that in an ELAN File
    std::vector<std::vector<double>> ChannelDataToWrite;
    std::vector<std::pair<int, int>> posSampleCodeToWrite;

    for(int i = 0; i < bigData.size(); i++)
    {
        std::vector<double> statToWrite;
        std::vector<std::pair<int, int>> posSampleCode;
        int sampleAlreadyWritten = 0;

        double sumIsChannelReactive = 0;
        std::vector<std::tuple<int, int, int>> CodeAndTrialsIndexes = triggerContainer->CodeAndTrialsIndexes();
        for(int j = 0; j < static_cast<int>(myprovFile->Blocs().size()); j++)
        {
            std::vector<int> codes = myprovFile->Blocs()[j].MainSubBloc().MainEvent().Codes();
            int windowBegin = (((float)myeegContainer->DownsampledFrequency() * myprovFile->Blocs()[j].MainSubBloc().MainWindow().Start()) / 1000);

            for(int k = 0; k < 3; k++) //repeat for better visibility in hibop
            {
                for(int l = 0; l < Stat_Z_CCS[i][j].size(); l++)
                {
                    statToWrite.push_back(10 * Stat_Z_CCS[i][j][l]);
                }

                int sample = std::abs(windowBegin) + sampleAlreadyWritten;
                int code = codes[0];
                posSampleCode.push_back(std::make_pair(sample,code));
                sampleAlreadyWritten += Stat_Z_CCS[i][j].size();
            }

            std::vector<int> indices;
            auto it = significantValue.begin();
            while ((it = std::find_if(it, significantValue.end(),[&](PVALUECOORD const& obj){ return obj.elec == i && obj.condit == j; })) != significantValue.end())
            {
                indices.push_back(std::distance(significantValue.begin(), it));
                it++;
            }

            //tout les indices correspondant dans le vector
            std::vector<double> signif = std::vector<double>(Stat_Z_CCS[i][j].size(), 0);
            for(int k = 0; k < indices.size(); k++)
            {
                int indexSignif = significantValue[indices[k]].window;
                signif[indexSignif] = 10 * Stat_Z_CCS[i][j][indexSignif];
                sumIsChannelReactive += signif[indexSignif];
            }

            for (int k = 0; k < 3; k++) //repeat for better visibility in hibop
            {
                for (int l = 0; l < signif.size(); l++)
                {
                    statToWrite.push_back(signif[l]);
                }
                int sample = std::abs(windowBegin) + sampleAlreadyWritten;
                int code = 1000 + codes[0];
                posSampleCode.push_back(std::make_pair(sample, code));
                sampleAlreadyWritten += signif.size();
            }
        }

        //ensuite mettre les 9999
        int windowBegin = (((float)myeegContainer->DownsampledFrequency() * myprovFile->Blocs()[0].MainSubBloc().MainWindow().Start()) / 1000);
        int valueToWrite = 30 * (sumIsChannelReactive > 0 ? 1 : 0);
        for (int j = 0; j < 3; j++) //repeat for better visibility in hibop
        {
            for (int k = 0; k < Stat_Z_CCS[0][0].size(); k++)
            {
                statToWrite.push_back(valueToWrite);
            }
            int sample = std::abs(windowBegin) + sampleAlreadyWritten;
            int code = 9999;
            posSampleCode.push_back(std::make_pair(sample, code));
            sampleAlreadyWritten += Stat_Z_CCS[0][0].size();
        }

        //need to add kruskall data to writable data
        int bigCounter = 0;
        for (int j = 0; j < static_cast<int>(myprovFile->Blocs().size() - 1); j++)
        {
            int counter = 0;
            for (int k = j + 1; k < static_cast<int>(myprovFile->Blocs().size()); k++)
            {
                std::vector<int> indices;
                auto it = significantValue2.begin();
                while ((it = std::find_if(it, significantValue2.end(), [&](PVALUECOORD_KW const& obj) { return obj.elec == i && obj.condit1 == j && obj.condit2 == counter; })) != significantValue2.end())
                {
                    indices.push_back(std::distance(significantValue2.begin(), it));
                    it++;
                }

                std::vector<double> signif = std::vector<double>(v_stat_K4[0][0][0].size(), 0);
                for (int m = 0; m < indices.size(); m++)
                {
                    int ind1 = significantValue2[indices[m]].elec;
                    int ind2 = significantValue2[indices[m]].condit1;
                    int ind3 = significantValue2[indices[m]].condit2;
                    int ind4 = significantValue2[indices[m]].window;

                    signif[ind4] = 30 * v_stat_K4[ind1][ind2][ind3][ind4];
                }

                for (int n = 0; n < 3; n++)
                {
                    for (int l = 0; l < signif.size(); l++)
                    {
                        statToWrite.push_back(signif[l]);
                    }

                    int sample = std::abs(windowBegin) + sampleAlreadyWritten;
                    int code = 10000 * (1000 + codesPairs[bigCounter].first) + (1000 + codesPairs[bigCounter].second);
                    posSampleCode.push_back(std::make_pair(sample, code));
                    sampleAlreadyWritten += signif.size();
                }

                counter++;
                bigCounter++;
            }
        }

        //eeg data
        ChannelDataToWrite.push_back(statToWrite);

        //copy pos data only once
        if (posSampleCodeToWrite.size() == 0)
        {
            posSampleCodeToWrite = std::vector<std::pair<int, int>>(posSampleCode);
        }
    }

    EEGFormat::ElanFile *outputFile = LoadDataInStructure(ChannelDataToWrite, myeegContainer);
    std::vector<std::string> filesPath = DefinePathForFiles(myeegContainer, smoothingID, myprovFile, freqFolder);
    WriteResultFile(outputFile, filesPath, posSampleCodeToWrite);

    //Delete what needs to be deleted
    delete[] windowSam;
}

std::vector<InsermLibrary::PVALUECOORD> InsermLibrary::StatisticalFilesProcessor::loadPValues(vec3<double>& pValues3D)
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
                pValues.push_back(tempPValue);
                count++;
            }
        }
    }

    return pValues;
}

std::vector<InsermLibrary::PVALUECOORD> InsermLibrary::StatisticalFilesProcessor::loadPValues(InsermLibrary::vec3<double>& pValues3D, float pLimit)
{
    int count = 0;
    InsermLibrary::PVALUECOORD tempPValue;
    std::vector<InsermLibrary::PVALUECOORD> pValues;

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
                    //tempPValue.weight = pSign3D[i][j][k];

                    pValues.push_back(tempPValue);
                }
                count++;
            }
        }
    }

    return pValues;
}

std::vector<InsermLibrary::PVALUECOORD_KW> InsermLibrary::StatisticalFilesProcessor::loadPValues_KW(vec4<double>& pValues4D)
{
    int count = 0;
    InsermLibrary::PVALUECOORD_KW tempPValue;
    std::vector<InsermLibrary::PVALUECOORD_KW> pValues;

    for (int i = 0; i < static_cast<int>(pValues4D.size()); i++)
    {
        for (int j = 0; j < static_cast<int>(pValues4D[i].size()); j++)
        {
            for (int k = 0; k < static_cast<int>(pValues4D[i][j].size()); k++)
            {
                for (int l = 0; l < static_cast<int>(pValues4D[i][j][k].size()); l++)
                {
                    tempPValue.elec = i;
                    tempPValue.condit1 = j;
                    tempPValue.condit2 = k;
                    tempPValue.window = l;
                    tempPValue.vectorpos = count;
                    tempPValue.pValue = pValues4D[i][j][k][l];
                    pValues.push_back(tempPValue);
                    count++;
                }
            }
        }
    }

    return pValues;
}

std::vector<InsermLibrary::PVALUECOORD_KW> InsermLibrary::StatisticalFilesProcessor::loadPValues_KW(vec4<double>& pValues4D, float pLimit)
{
    int count = 0;
    InsermLibrary::PVALUECOORD_KW tempPValue;
    std::vector<InsermLibrary::PVALUECOORD_KW> pValues;

    for (int i = 0; i < static_cast<int>(pValues4D.size()); i++)
    {
        for (int j = 0; j < static_cast<int>(pValues4D[i].size()); j++)
        {
            for (int k = 0; k < static_cast<int>(pValues4D[i][j].size()); k++)
            {
                for (int l = 0; l < static_cast<int>(pValues4D[i][j][k].size()); l++)
                {
                    if (pValues4D[i][j][k][l] < pLimit)
                    {
                        tempPValue.elec = i;
                        tempPValue.condit1 = j;
                        tempPValue.condit2 = k;
                        tempPValue.window = l;
                        tempPValue.vectorpos = count;
                        tempPValue.pValue = pValues4D[i][j][k][l];
                        pValues.push_back(tempPValue);
                        count++;
                    }
                }
            }
        }
    }

    return pValues;
}

EEGFormat::ElanFile* InsermLibrary::StatisticalFilesProcessor::LoadDataInStructure(std::vector<std::vector<double>> ChannelDataToWrite, eegContainer* eegContainer)
{
    EEGFormat::ElanFile *outputFile = new EEGFormat::ElanFile();
    outputFile->ElectrodeCount((int)ChannelDataToWrite.size());
    outputFile->SamplingFrequency(eegContainer->DownsampledFrequency());
    //Load electrodes list according to container
    std::vector<EEGFormat::IElectrode*> bipolesList;
    int BipoleCount = eegContainer->BipoleCount();
    for (int i = 0; i < BipoleCount; i++)
    {
        std::pair<int, int> currentBipole = eegContainer->Bipole(i);
        bipolesList.push_back(eegContainer->Electrode(currentBipole.first));
    }
    outputFile->Electrodes(bipolesList);
    //Define type of elec : label + "EEG" + "uV"
    outputFile->Data(EEGFormat::DataConverterType::Digital).resize((int)bipolesList.size(), std::vector<float>(ChannelDataToWrite[0].size() + 10));
    for (int i = 0; i < ChannelDataToWrite.size(); i++)
    {
        for (int j = 0; j < ChannelDataToWrite[i].size() - 10; j++)
        {
            outputFile->Data(EEGFormat::DataConverterType::Digital)[i][j] = ChannelDataToWrite[i][j];
        }
    }

    return outputFile;
}

std::vector<std::string> InsermLibrary::StatisticalFilesProcessor::DefinePathForFiles(eegContainer* eegContainer, int smoothingID, ProvFile* myprovFile, std::string freqFolder)
{
    std::vector<std::string> filesPath;

    std::string smoothing = (smoothingID == 0) ? "sm0" : (smoothingID == 1) ? "sm250" : (smoothingID == 2) ? "sm500" : (smoothingID == 3) ? "sm1000" : (smoothingID == 4) ? "sm2500" : "sm5000";

    std::string patientName = eegContainer->RootFileName();
    vec1<std::string> patientNameSplit = split<std::string>(patientName, "_");
    patientName = patientNameSplit[0];

    vec1<std::string> pathSplit = split<std::string>(freqFolder, "/");
    std::string frequencyFolder = pathSplit[pathSplit.size() - 1];

    //If this is not the prov for the task and some other way to visualise data, we need to precise it in the file name
    std::string suffix = myprovFile->Name();
//    suffix = std::regex_replace(suffix, std::regex("_STATISTICS"), "");
//    if(pathSplit[pathSplit.size() - 1].find(suffix) != std::string::npos)
//    {
//        suffix = "";
//    }
//    else
//    {
//        suffix = "_" + suffix;
//    }

    std::string directory = eegContainer->RootOutputFileFolder();
    std::string baseName = patientName + "_task-" + suffix + "_acq-" + frequencyFolder + "ds" + std::to_string(eegContainer->DownsamplingFactor()) + smoothing + "_ieeg";

    filesPath.push_back(directory + frequencyFolder + "/" + baseName + ".eeg.ent");
    filesPath.push_back(directory + frequencyFolder + "/" + baseName + ".eeg");
    filesPath.push_back(directory + baseName + ".pos");
    return filesPath;
}

void InsermLibrary::StatisticalFilesProcessor::WriteResultFile(EEGFormat::ElanFile* outputFile, std::vector<std::string> filesPath, std::vector<std::pair<int, int>> posSampleCodeToWrite)
{
    //Save ent and eeg
    outputFile->SaveAs(filesPath[0], filesPath[1], "","");
    //then triggers in pos
    std::vector<EEGFormat::ITrigger> iTriggers(posSampleCodeToWrite.size());
    for (int i = 0; i < posSampleCodeToWrite.size(); i++)
    {
        int code = posSampleCodeToWrite[i].second;
        long sample = posSampleCodeToWrite[i].first;
        iTriggers[i] = EEGFormat::ElanTrigger(code, sample);
    }
    EEGFormat::ElanFile::SaveTriggers(filesPath[2], iTriggers);
    //and delete pointer
    DeleteGenericFile(outputFile);
}
