#include "TrialMatricesProcessor.h"
#include <QDir>
#include "../../Framework/Framework/Measure.h"
#include "mapsGenerator.h"
#include "barsPlotsGenerator.h"

void InsermLibrary::TrialMatricesProcessor::Process(TriggerContainer* triggerContainer, eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder, statOption* statOption, picOption* picOption)
{
    std::vector<PVALUECOORD> significantValue;
    //== get some useful information
    std::string mapsFolder = GetTrialmatFolder(myprovFile, freqFolder, statOption);
    std::string mapPath = PrepareFolderAndPathsTrial(mapsFolder, myeegContainer);

    // Get biggest window possible, for now we use the assumption that every bloc has the same window
    // TODO : deal with possible different windows
    int StartInSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().Start() * myeegContainer->DownsampledFrequency()) / 1000;
    int EndinSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().End() * myeegContainer->DownsampledFrequency()) / 1000;
    int* windowSam = new int[2]{ StartInSam, EndinSam };

    //== get Bloc of eeg data we want to display center around events
    vec3<float> bigData;
    bigData.resize(myeegContainer->BipoleCount(), vec2<float>(triggerContainer->ProcessedTriggerCount(), vec1<float>(windowSam[1] - windowSam[0])));
    myeegContainer->GetFrequencyBlocData(bigData, 0, triggerContainer->ProcessedTriggers(), windowSam);

    //== calculate stat
    significantValue = ProcessWilcoxonStatistic(bigData, triggerContainer, myeegContainer, myprovFile, mapsFolder, statOption);

    //== Draw for each plot and according to a template to reduce drawing time
    std::vector<std::tuple<int, int, int>> CodeAndTrialsIndexes = triggerContainer->CodeAndTrialsIndexes();
    InsermLibrary::DrawCard::mapsGenerator mGen(picOption->sizeTrialmap.width(), picOption->sizeTrialmap.height());
    mGen.trialmatTemplate(CodeAndTrialsIndexes, myprovFile);

    QPixmap* pixmapChanel = nullptr, * pixmapSubSubMatrix = nullptr;
    QPainter* painterChanel = nullptr, * painterSubSubMatrix = nullptr;
    for (int i = 0; i < myeegContainer->elanFrequencyBand[0]->ElectrodeCount(); i++)
    {
        deleteAndNullify1D(painterChanel);
        deleteAndNullify1D(pixmapChanel);
        pixmapChanel = new QPixmap(mGen.pixmapTemplate);
        painterChanel = new QPainter(pixmapChanel);

        float stdRes = Framework::Calculations::Stats::Measure::MeanOfStandardDeviation(bigData[i]);
        float Maxi = 2.0 * abs(stdRes);
        float Mini = -2.0 * abs(stdRes);
        float AbsMax = std::fmax(abs(Maxi), abs(Mini));
        mGen.graduateColorBar(painterChanel, AbsMax);

        int interpolFactorX = picOption->interpolationtrialmap.width();
        int interpolFactorY = picOption->interpolationtrialmap.height();

        int conditionCount = static_cast<int>(CodeAndTrialsIndexes.size());
        for (int j = 0; j < conditionCount; j++)
        {
            int index = -1;
            for (int k = 0; k < myprovFile->Blocs().size(); k++)
            {
                if (myprovFile->Blocs()[k].MainSubBloc().MainEvent().Codes()[0] == std::get<0>(CodeAndTrialsIndexes[j]))
                {
                    index = k;
                    break;
                }
            }

            //TODO : probably check that we find an index, we should but who knows
            int StartInMs = myprovFile->Blocs()[index].MainSubBloc().MainWindow().Start();
            int EndInMs = myprovFile->Blocs()[index].MainSubBloc().MainWindow().End();
            int* currentWinMs = new int[2]{ StartInMs, EndInMs };
            //=====
            int StartInSam = (StartInMs * myeegContainer->DownsampledFrequency()) / 1000;
            int EndinSam = (EndInMs * myeegContainer->DownsampledFrequency()) / 1000;
            int* currentWinSam = new int[2]{ StartInSam, EndinSam };
            //=====
            int nbSampleWindow = currentWinSam[1] - currentWinSam[0];
            int indexBegTrigg = std::get<1>(CodeAndTrialsIndexes[j]);
            int numberSubTrial = std::get<2>(CodeAndTrialsIndexes[j]) - indexBegTrigg;
            if(numberSubTrial > 0)
            {
                int subsubMatrixHeigth = 0;

                vec1<int> colorX[512], colorY[512];
                if (interpolFactorX > 1)
                {
                    vec2<float> dataInterpolatedHoriz = mGen.horizontalInterpolation(bigData[i], interpolFactorX, indexBegTrigg, numberSubTrial);
                    vec2<float> dataInterpolatedVerti = mGen.verticalInterpolation(dataInterpolatedHoriz, interpolFactorY);
                    mGen.eegData2ColorMap(colorX, colorY, dataInterpolatedVerti, AbsMax);
                    subsubMatrixHeigth = interpolFactorY * (numberSubTrial - 1);
                }
                else
                {
                    mGen.eegData2ColorMap(colorX, colorY, bigData[i], AbsMax);
                    subsubMatrixHeigth = numberSubTrial;
                }

                /***************************************************/
                /*	ceate subBlock and paste it on the big Matrix  */
                /***************************************************/
                deleteAndNullify1D(painterSubSubMatrix);
                deleteAndNullify1D(pixmapSubSubMatrix);
                pixmapSubSubMatrix = new QPixmap(interpolFactorX * nbSampleWindow, subsubMatrixHeigth);
                painterSubSubMatrix = new QPainter(pixmapSubSubMatrix);
                painterSubSubMatrix->setBackgroundMode(Qt::BGMode::TransparentMode);

                for (int k = 0; k < 512; k++)
                {
                    painterSubSubMatrix->setPen(QColor(mGen.ColorMapJet[k].red(), mGen.ColorMapJet[k].green(), mGen.ColorMapJet[k].blue()));

                    int colorCount = static_cast<int>(colorX[k].size());
                    for (int l = 0; l < colorCount; l++)
                    {
                        painterSubSubMatrix->drawPoint(QPoint(colorX[k][l], subsubMatrixHeigth - colorY[k][l]));
                    }
                }

                auto itttt = std::find_if(mGen.subMatrixesCodes.begin(), mGen.subMatrixesCodes.end(), [&](const int& c)
                {
                    return c == std::get<0>(CodeAndTrialsIndexes[j]);
                });
                int indexPos = std::distance(mGen.subMatrixesCodes.begin(), itttt);
                painterChanel->drawPixmap(mGen.subMatrixes[indexPos].x(), mGen.subMatrixes[indexPos].y(),
                                          pixmapSubSubMatrix->scaled(QSize(mGen.subMatrixes[indexPos].width(), mGen.subMatrixes[indexPos].height()),
                                                                     Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation));

                /********************************************************************/
                /*	Add reaction time on map, if the calculated position is not		*/
                /*  inside the map , no need to draw it							 	*/
                /********************************************************************/
                painterChanel->setPen(QColor(Qt::black));
                for (int l = 0; l < numberSubTrial; l++)
                {
                    int xReactionTimeMs = abs(currentWinMs[0]) + triggerContainer->ProcessedTriggers()[indexBegTrigg + l].ReactionTimeInMilliSeconds();
                    double xScale = (double)(currentWinMs[1] - currentWinMs[0]) / mGen.MatrixRect.width();
                    double xRt = mGen.MatrixRect.x() + (xReactionTimeMs / xScale);

                    int yTrialPosition = mGen.subMatrixes[indexPos].y() + mGen.subMatrixes[indexPos].height();
                    double yRt = yTrialPosition - (((double)mGen.subMatrixes[indexPos].height() / numberSubTrial) * l) - 1;

                    if (xRt >= mGen.MatrixRect.x() && xRt <= (mGen.MatrixRect.x() + mGen.MatrixRect.width()))
                    {
                        painterChanel->setBrush(Qt::black);
                        painterChanel->drawEllipse(QPoint(xRt, yRt), (int)(0.0034722 * mGen.MatrixRect.width()),
                                                   (int)(0.004629629 * mGen.MatrixRect.height()));
                    }
                }

            }
            delete[] currentWinMs;
            delete[] currentWinSam;
        }

        mGen.drawVerticalZeroLine(painterChanel, myprovFile);

        //Display Stat
        if (statOption->wilcoxon)
        {
            std::vector<int> allIdCurrentMap = mGen.checkIfNeedDisplayStat(significantValue, i);
            if (allIdCurrentMap.size() > 0)
            {
                mGen.displayStatsOnMap(painterChanel, significantValue, i, myprovFile);
            }
        }

        //Display title on map and then Save
        std::string outputPicPath = mapPath;
        std::string elecName = myeegContainer->flatElectrodes[myeegContainer->Bipole(i).first];
        outputPicPath.append(elecName.c_str()).append(".jpg");
        mGen.drawMapTitle(painterChanel, outputPicPath);
        pixmapChanel->save(outputPicPath.c_str(), "JPG");
    }

    delete[] windowSam;
    deleteAndNullify1D(painterChanel);
    deleteAndNullify1D(pixmapChanel);
    deleteAndNullify1D(painterSubSubMatrix);
    deleteAndNullify1D(pixmapSubSubMatrix);
}

std::string InsermLibrary::TrialMatricesProcessor::GetTrialmatFolder(ProvFile* myprovFile, std::string freqFolder, statOption* statOption)
{
    std::string mapsFolder = freqFolder;
    vec1<std::string> dd = split<std::string>(mapsFolder, "/");
    mapsFolder.append(dd[dd.size() - 1]);

    if (myprovFile->FilePath().find("INVERTED") != std::string::npos)
    {
        mapsFolder.append("_trials_resp");	//if inverted is found in the name
    }
    else
    {
        mapsFolder.append("_trials_stim");
    }

    std::stringstream streamPValue;
    streamPValue << std::fixed << std::setprecision(2) << statOption->pWilcoxon;
    if (statOption->wilcoxon)
    {
        if (statOption->FDRwilcoxon)
            mapsFolder.append("_FDR" + streamPValue.str());
        else
            mapsFolder.append("_P" + streamPValue.str());
    }

    std::vector<std::string> myProv = split<std::string>(myprovFile->FilePath(), "/");
    mapsFolder.append(" - " + myProv[myProv.size() - 1]);

    return mapsFolder;
}

std::string InsermLibrary::TrialMatricesProcessor::PrepareFolderAndPathsTrial(std::string mapsFolder, eegContainer* myeegContainer)
{
    if (!QDir(&mapsFolder.c_str()[0]).exists())
        QDir().mkdir(&mapsFolder.c_str()[0]);

    vec1<std::string> pathSplit = split<std::string>(mapsFolder, "/");
    qDebug() << mapsFolder.c_str();
    vec1<std::string> frequencySuffixSplit =  split<std::string>(pathSplit[pathSplit.size() - 1], "_");

    vec1<std::string> patientNameSplit = split<std::string>(myeegContainer->RootFileName(), "_");
    std::string sub = patientNameSplit[0];
    std::string task = patientNameSplit[2];

    std::string fileNameBase =  myeegContainer->RootOutputFileFolder() + sub + "_" + task;
    std::string labelName = sub + "_" + task + "_acq-" + frequencySuffixSplit[0] + "ds" +   std::to_string(myeegContainer->DownsamplingFactor());

    return std::string(mapsFolder + "/" + labelName + "sm0trials_");
}

InsermLibrary::vec1<InsermLibrary::PVALUECOORD> InsermLibrary::TrialMatricesProcessor::ProcessWilcoxonStatistic(vec3<float>& bigData, TriggerContainer* triggerContainer, eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder, statOption* statOption)
{
    std::vector<PVALUECOORD> significantValue;
    if (statOption->wilcoxon)
    {
        int copyIndex = 0;
        vec3<float> pValue3D; vec3<int> psign3D;
        Stats::pValuesWilcoxon(pValue3D, psign3D, bigData, triggerContainer, myeegContainer->DownsampledFrequency(), myprovFile);
        if (statOption->FDRwilcoxon)
        {
            significantValue = Stats::FDR(pValue3D, psign3D, copyIndex, statOption->pWilcoxon);
        }
        else
        {
            significantValue = Stats::loadPValues(pValue3D, psign3D, statOption->pWilcoxon);
        }
        Stats::exportStatsData(myeegContainer, myprovFile, significantValue, freqFolder, false);
    }

    return significantValue;
}
