#include "BarplotProcessor.h"

#include "mapsGenerator.h"
#include "barsPlotsGenerator.h"

void InsermLibrary::BarplotProcessor::Process(TriggerContainer* triggerContainer, eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder, statOption* statOption, picOption* picOption)
{
	std::string mapsFolder = GetBarplotMapsFolder(freqFolder, myprovFile, statOption);
	std::string mapPath = PrepareFolderAndPathsBar(mapsFolder, myeegContainer->DownsamplingFactor());
	// Get biggest window possible, for now we use the assumption that every bloc has the same window
	// TODO : deal with possible different windows
	int StartInSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().Start() * myeegContainer->DownsampledFrequency()) / 1000;
	int EndinSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().End() * myeegContainer->DownsampledFrequency()) / 1000;
	int* windowSam = new int[2]{ StartInSam, EndinSam };

	//== get Bloc of eeg data we want to display center around events
	vec3<float> eegData3D = vec3<float>(triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocDataEvents(eegData3D, 0, triggerContainer->ProcessedTriggers(), windowSam);

	//== calculate stat
	vec1<PVALUECOORD> significantValue = ProcessKruskallStatistic(eegData3D, triggerContainer, myeegContainer, myprovFile, mapsFolder, statOption);

	//==
	InsermLibrary::DrawbarsPlots::drawBars b = InsermLibrary::DrawbarsPlots::drawBars(myprovFile, mapPath, picOption->sizePlotmap);
	b.drawDataOnTemplate(eegData3D, triggerContainer, significantValue, myeegContainer);

	delete[] windowSam;
}

std::string InsermLibrary::BarplotProcessor::GetBarplotMapsFolder(std::string freqFolder, ProvFile* myprovFile, statOption* statOption)
{
	std::string mapsFolder = freqFolder;
	vec1<std::string> dd = split<std::string>(mapsFolder, "/");
	mapsFolder.append(dd[dd.size() - 1]);

	mapsFolder.append("_bar");

	std::stringstream streamPValue;
	streamPValue << std::fixed << std::setprecision(2) << statOption->pKruskall;
	if (statOption->kruskall)
	{
		if (statOption->FDRkruskall)
			mapsFolder.append("_FDR" + streamPValue.str());
		else
			mapsFolder.append("_P" + streamPValue.str());
	}

	std::vector<std::string> myProv = split<std::string>(myprovFile->FilePath(), "/");
	mapsFolder.append(" - " + myProv[myProv.size() - 1]);

	return mapsFolder;
}

std::string InsermLibrary::BarplotProcessor::PrepareFolderAndPathsBar(std::string mapsFolder, int dsSampFreq)
{
	if (!std::filesystem::exists(mapsFolder))
	{
		std::filesystem::create_directory(mapsFolder);
	}

	vec1<std::string> dd = split<std::string>(mapsFolder, "/");
	return std::string(mapsFolder + "/" + dd[dd.size() - 2] + "_ds" + std::to_string(dsSampFreq) + "_sm0_bar_");
}

InsermLibrary::vec1<InsermLibrary::PVALUECOORD> InsermLibrary::BarplotProcessor::ProcessKruskallStatistic(vec3<float>& bigData, TriggerContainer* triggerContainer, eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder, statOption* statOption)
{
	std::vector<PVALUECOORD> significantValue;
	if (statOption->kruskall)
	{
		int copyIndex = 0;
		vec3<float> pValue3D; vec3<int> psign3D;
		Stats::pValuesKruskall(pValue3D, psign3D, bigData, triggerContainer, myeegContainer->DownsampledFrequency(), myprovFile);
		if (statOption->FDRkruskall)
		{
			significantValue = Stats::FDR(pValue3D, psign3D, copyIndex, statOption->pKruskall);
		}
		else
		{
			significantValue = Stats::loadPValues(pValue3D, psign3D, statOption->pKruskall);
		}
		Stats::exportStatsData(myeegContainer, myprovFile, significantValue, freqFolder, true);
	}

	return significantValue;
}
