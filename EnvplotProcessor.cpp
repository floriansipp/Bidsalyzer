#include "EnvplotProcessor.h"
#include "mapsGenerator.h"
#include "barsPlotsGenerator.h"
#include <QDir>

/************/
/* Env2Plot */
/************/
void InsermLibrary::EnvplotProcessor::Process(TriggerContainer* triggerContainer, eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder, picOption* picOption)
{
	std::string mapsFolder = GetEnv2PlotMapsFolder(freqFolder, myprovFile);
	std::string mapPath = PrepareFolderAndPathsPlot(mapsFolder, myeegContainer->DownsamplingFactor());
	// Get biggest window possible, for now we use the assumption that every bloc has the same window
	// TODO : deal with possible different windows
	int StartInSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().Start() * myeegContainer->DownsampledFrequency()) / 1000;
	int EndinSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().End() * myeegContainer->DownsampledFrequency()) / 1000;
	int* windowSam = new int[2]{ StartInSam, EndinSam };

	//== get Bloc of eeg data we want to display center around events
	vec3<float> eegData3D = vec3<float>(triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocDataEvents(eegData3D, 0, triggerContainer->ProcessedTriggers(), windowSam);

	//==
	InsermLibrary::DrawbarsPlots::drawPlots b = InsermLibrary::DrawbarsPlots::drawPlots(myprovFile, mapPath, picOption->sizePlotmap);
	b.drawDataOnTemplate(eegData3D, triggerContainer, myeegContainer, 2);

	delete[] windowSam;
}

std::string InsermLibrary::EnvplotProcessor::GetEnv2PlotMapsFolder(std::string freqFolder, ProvFile* myprovFile)
{
	std::string mapsFolder = freqFolder;
	vec1<std::string> dd = split<std::string>(mapsFolder, "/");
	mapsFolder.append(dd[dd.size() - 1]);

	std::vector<std::string> myProv = split<std::string>(myprovFile->FilePath(), "/");
	return mapsFolder.append("_plots").append(" - " + myProv[myProv.size() - 1]);
}

std::string InsermLibrary::EnvplotProcessor::PrepareFolderAndPathsPlot(std::string mapsFolder, int dsSampFreq)
{
	if (!QDir(&mapsFolder.c_str()[0]).exists())
		QDir().mkdir(&mapsFolder.c_str()[0]);

	vec1<std::string> dd = split<std::string>(mapsFolder, "/");

	return std::string(mapsFolder + "/" + dd[dd.size() - 2] + "_ds" + std::to_string(dsSampFreq) + "_sm0_plot_");
}