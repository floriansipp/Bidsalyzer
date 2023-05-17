#include "LOCA.h"
#include <filesystem>

InsermLibrary::LOCA::LOCA(std::vector<FrequencyBandAnalysisOpt>& analysisOpt, statOption* statOption, picOption* picOption, std::string ptsFilePath)
{
	m_analysisOpt = analysisOpt;
	m_statOption = statOption;
	m_picOption = picOption;
	m_PtsFilePath = ptsFilePath;
}

InsermLibrary::LOCA::~LOCA()
{
	EEGFormat::Utility::DeleteAndNullify(m_triggerContainer);
	EEGFormat::Utility::DeleteAndNullify(m_statOption);
	EEGFormat::Utility::DeleteAndNullify(m_picOption);
}

/************/
/* eeg2erp  */
/************/
void InsermLibrary::LOCA::Eeg2erp(eegContainer* myeegContainer, ProvFile* myprovFile)
{
	// Get biggest window possible, for now we use the assumption that every bloc has the same window
	// TODO : deal with possible different windows
	int StartInSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().Start() * myeegContainer->SamplingFrequency()) / 1000;
	int EndinSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().End() * myeegContainer->SamplingFrequency()) / 1000;
	int* windowSam = new int[2]{ StartInSam, EndinSam };

	std::string outputErpFolder = myeegContainer->RootFileFolder();
	outputErpFolder.append(myeegContainer->RootFileName());
	outputErpFolder.append("_ERP/");
	if (!QDir(&outputErpFolder.c_str()[0]).exists())
	{
		emit sendLogInfo(QString::fromStdString("Creating Output Folder for erp Maps"));
		QDir().mkdir(&outputErpFolder.c_str()[0]);
	}
	std::string monoErpOutput = outputErpFolder.append(myeegContainer->RootFileName());

	deleteAndNullify1D(m_triggerContainer);
	std::vector<EEGFormat::ITrigger> triggers = myeegContainer->Triggers();
	int samplingFrequency = myeegContainer->SamplingFrequency();
	m_triggerContainer = new TriggerContainer(triggers, samplingFrequency);
    m_triggerContainer->ProcessEventsForExperiment(myprovFile, 99, myeegContainer->DownsamplingFactor());

	vec3<float> bigDataMono = vec3<float>(m_triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->flatElectrodes.size(), vec1<float>(windowSam[1] - windowSam[0])));
	vec3<float> bigDataBipo = vec3<float>(m_triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));

	std::cout << "Reading Data Mono ..." << std::endl;
	for (int i = 0; i < m_triggerContainer->ProcessedTriggerCount(); i++)
	{
		for (int j = 0; j < (windowSam[1] - windowSam[0]); j++)
		{
			for (int k = 0; k < myeegContainer->flatElectrodes.size(); k++)
			{
				int beginPos = m_triggerContainer->ProcessedTriggers()[i].MainSample() + windowSam[0];
				bigDataMono[i][k][j] = myeegContainer->Data()[k][beginPos + j];
			}
		}
	}

	std::cout << "Reading Data Bipo ..." << std::endl;
	for (int i = 0; i < m_triggerContainer->ProcessedTriggerCount(); i++)
	{
		for (int k = 0; k < myeegContainer->BipoleCount(); k++)
		{
			for (int j = 0; j < (windowSam[1] - windowSam[0]); j++)
			{
				bigDataBipo[i][k][j] = bigDataMono[i][myeegContainer->Bipole(k).first][j] -
					bigDataMono[i][myeegContainer->Bipole(k).second][j];
			}
		}
	}

	InsermLibrary::DrawbarsPlots::drawPlots b = InsermLibrary::DrawbarsPlots::drawPlots(myprovFile, monoErpOutput, m_picOption->sizePlotmap);
	b.drawDataOnTemplate(bigDataMono, m_triggerContainer, myeegContainer, 0);
	emit sendLogInfo("Mono Maps Generated");
	b.drawDataOnTemplate(bigDataBipo, m_triggerContainer, myeegContainer, 1);
	emit sendLogInfo("Bipo Maps Generated");
	emit incrementAdavnce(1);

	delete[] windowSam;
}

void InsermLibrary::LOCA::Localize(eegContainer* myeegContainer, int idCurrentLoca,  InsermLibrary::IEegFileInfo* currentLoca, std::string taskName)
{
	m_idCurrentLoca = idCurrentLoca;
    //m_currentLoca = currentLoca;
    m_currentLocaName = taskName;

	int examCountToProcess = static_cast<int>(m_analysisOpt.size());
	for (int i = 0; i < examCountToProcess; i++)
	{
		FrequencyBand currentFrequencyBand(m_analysisOpt[i].Band);
		if (m_analysisOpt[i].analysisParameters.eeg2env2)
		{
			currentFrequencyBand.CheckShannonCompliance(myeegContainer->SamplingFrequency());

			Algorithm::AlgorithmCalculator::ExecuteAlgorithm(m_analysisOpt[i].analysisParameters.calculationType, myeegContainer, currentFrequencyBand.FrequencyBins());
			myeegContainer->SaveFrequencyData(m_analysisOpt[i].analysisParameters.outputType, currentFrequencyBand.FrequencyBins());
			emit incrementAdavnce(1);
			emit sendLogInfo("Hilbert Envelloppe Calculated");

			std::string freqFolder = CreateFrequencyFolder(myeegContainer, currentFrequencyBand);
			GenerateMapsAndFigures(myeegContainer, freqFolder, m_analysisOpt[i]);
		}
		else
		{
            std::vector<std::pair<std::string, std::string>> frequencyOutputFolders;
            for (const auto & entry : std::filesystem::directory_iterator(myeegContainer->RootOutputFileFolder()))
            {
                if(std::filesystem::is_directory(entry))
                {
                    std::string path = entry.path().string();
                    std::string key = entry.path().filename().string();
                    //std::cout << path << " and " << key << std::endl;
                    frequencyOutputFolders.push_back(std::make_pair(key, path));
                }
            }

            int frequencyCount = static_cast<int>(frequencyOutputFolders.size());
            for (int j = 0; j < frequencyCount; j++)
            {
                //TODO : Need to compare frequencyOutputFolders informations with current frequencyToSearch in
                //      order to prompt a message in case there is no env data
                std::string fMin = std::to_string(currentFrequencyBand.FMin());
                std::string fMax = std::to_string(currentFrequencyBand.FMax());
                std::string frequencyToSearch  ="f" + fMin + "f" + fMax;

                std::cout << frequencyToSearch << std::endl;

                int loadedCount = 0;

                loadedCount += LoadData(myeegContainer, frequencyOutputFolders[j], frequencyToSearch, SmoothingWindow::SM0, 0);
                loadedCount += LoadData(myeegContainer, frequencyOutputFolders[j], frequencyToSearch, SmoothingWindow::SM250, 1);
                loadedCount += LoadData(myeegContainer, frequencyOutputFolders[j], frequencyToSearch, SmoothingWindow::SM500, 2);
                loadedCount += LoadData(myeegContainer, frequencyOutputFolders[j], frequencyToSearch, SmoothingWindow::SM1000, 3);
                loadedCount += LoadData(myeegContainer, frequencyOutputFolders[j], frequencyToSearch, SmoothingWindow::SM2500, 4);
                loadedCount += LoadData(myeegContainer, frequencyOutputFolders[j], frequencyToSearch, SmoothingWindow::SM5000, 5);

                if (loadedCount > 0)
                {
                    emit incrementAdavnce(1);
                    std::string freqFolder = CreateFrequencyFolder(myeegContainer, currentFrequencyBand);
                    GenerateMapsAndFigures(myeegContainer, freqFolder, m_analysisOpt[i]);
                }
            }
		}
    }
}

void InsermLibrary::LOCA::LocalizeMapsOnly(eegContainer* myeegContainer, int idCurrentLoca)
{
	m_idCurrentLoca = idCurrentLoca;
	m_currentLoca = nullptr;
	int examCountToProcess = static_cast<int>(m_analysisOpt.size());

	std::vector<EEGFormat::ITrigger> triggers = myeegContainer->Triggers();
	int samplingFrequency = myeegContainer->SamplingFrequency();
	m_triggerContainer = new TriggerContainer(triggers, samplingFrequency);
	for (int i = 0; i < examCountToProcess; i++)
	{
		FrequencyBand currentFrequencyBand(m_analysisOpt[i].Band);
		currentFrequencyBand.CheckShannonCompliance(myeegContainer->SamplingFrequency());
		if (m_analysisOpt[i].analysisParameters.eeg2env2)
		{
			Algorithm::AlgorithmCalculator::ExecuteAlgorithm(m_analysisOpt[i].analysisParameters.calculationType, myeegContainer, currentFrequencyBand.FrequencyBins());
			myeegContainer->SaveFrequencyData(m_analysisOpt[i].analysisParameters.outputType, currentFrequencyBand.FrequencyBins());
			emit incrementAdavnce(1);
			emit sendLogInfo("Hilbert Envelloppe Calculated");

			CreateEventsFile(m_analysisOpt[i], myeegContainer, m_triggerContainer, nullptr);
		}
	}
	deleteAndNullify1D(m_triggerContainer);
}

InsermLibrary::IEegFileInfo* InsermLibrary::LOCA::GetEegFileInfo(eegContainer* myeegContainer, std::pair<std::string,std::string> kvp, SmoothingWindow smoothingWindow, int index, InsermLibrary::FileType fileType)
{
    std::string smKey = "NOT_DEFINED";
    switch (smoothingWindow)
    {
        case SmoothingWindow::SM0:
            smKey = "sm0";
            break;
        case SmoothingWindow::SM250:
            smKey = "sm250";
            break;
        case SmoothingWindow::SM500:
            smKey = "sm500";
            break;
        case SmoothingWindow::SM1000:
            smKey = "sm1000";
            break;
        case SmoothingWindow::SM2500:
            smKey = "sm2500";
            break;
        case SmoothingWindow::SM5000:
            smKey = "sm5000";
            break;
        default:
            smKey = "NOT_DEFINED";
            break;
    }

    for (auto &p : std::filesystem::recursive_directory_iterator(kvp.second))
    {
        if (p.path().extension() == ".vhdr")
        {
            if (p.path().string().find(kvp.first) != std::string::npos && p.path().string().find(smKey) != std::string::npos)
            {
//                std::cout << "found!" << '\n';
//                std::cout << p.path().stem().string() << '\n';
//                std::cout << p.path().string() << '\n';

                return new InsermLibrary::BrainVisionFileInfo(p.path().string());
            }
        }
    }

    return nullptr;
}

int InsermLibrary::LOCA::LoadData(eegContainer* myeegContainer, std::pair<std::string,std::string> kvp, std::string frequency, SmoothingWindow smoothingWindow, int index)
{
    IEegFileInfo *requestedFile = GetEegFileInfo(myeegContainer, kvp, smoothingWindow, index, InsermLibrary::FileType::Brainvision);
    if(requestedFile != nullptr)
    {
        if(kvp.first == frequency && requestedFile->CheckForErrors() == 0)
        {
            std::cout << "Should do " << kvp.second << std::endl;
            return LoadProcessedData(myeegContainer, requestedFile, index);
        }
    }
    return 0;
}

int InsermLibrary::LOCA::LoadProcessedData(eegContainer* myeegContainer, InsermLibrary::IEegFileInfo* requestedFile, int index)
{
    if(requestedFile->CheckForErrors() == 0)
    {
        std::vector<std::string> dataFiles = requestedFile->GetFiles();
        int result = myeegContainer->LoadFrequencyData(dataFiles, index);
        if (result == 0)
        {
            emit sendLogInfo("Envelloppe File Loaded at place number " + QString::number(index));
            return 1;
        }
        else
        {
            emit sendLogInfo("Problem loading file at place number " + QString::number(index));
            return 0;
        }
    }
    else
    {
        emit sendLogInfo("The data for this smoothing window / Filetype seems to have a problem");
        return 0;
    }
}

void InsermLibrary::LOCA::GenerateMapsAndFigures(eegContainer* myeegContainer, std::string freqFolder, FrequencyBandAnalysisOpt a)
{
    std::vector<EEGFormat::ITrigger> triggers = myeegContainer->Triggers();
    int samplingFrequency = myeegContainer->SamplingFrequency();
    m_triggerContainer = new TriggerContainer(triggers, samplingFrequency);

    //We generate file.pos and file_dsX.pos if we find a prov file
    //with the exact same name as the experiment.
    ProvFile* task = LoadProvForTask(m_currentLocaName);
    ProvFile* taskInverted = LoadProvForTask(m_currentLocaName, "INVERTED");
    ProvFile* taskBarPlot = LoadProvForTask(m_currentLocaName, "BARPLOT");
    ProvFile* taskStatistics = LoadProvForTask(m_currentLocaName, "STATISTICS");
    ProvFile* taskMaintenanceStatistics = LoadProvForTask(m_currentLocaName, "MAINTENANCE_STATISTICS");

    if (task != nullptr)
    {
        CreateEventsFile(a, myeegContainer, m_triggerContainer, task);
    }

    //Process Env2Plot => LOCA
    if (a.env2plot)
    {
        if(task != nullptr)
        {
            m_triggerContainer->ProcessEventsForExperiment(task, 99);
            if (m_triggerContainer->ProcessedTriggerCount() == 0)
            {
                emit sendLogInfo("No Trigger found for this experiment, aborting Env2Plot generation");
            }
            else
            {
				EnvplotProcessor epp;
				epp.Process(m_triggerContainer, myeegContainer, task, freqFolder, m_picOption);
                emit sendLogInfo("Envelope plots generated");
            }
            emit incrementAdavnce(1);
        }
    }

    //Process Trialmatrices => LOCA and LOCA_INVERTED and LOCA_BARPLOT (only AUDI)
   if (a.trialmat)
   {
       if(task != nullptr)
       {
            m_triggerContainer->ProcessEventsForExperiment(task, 99);
            if (m_triggerContainer->ProcessedTriggerCount() == 0)
            {
                emit sendLogInfo("No Trigger found for this experiment, aborting trialmats generation");
            }
            else
            {
				TrialMatricesProcessor tmp;
				tmp.Process(m_triggerContainer, myeegContainer, task, freqFolder, m_statOption, m_picOption);
				emit sendLogInfo("Time Trials Matrices generated");
            }
       }
       if(taskInverted != nullptr)
       {
			m_triggerContainer->ProcessEventsForExperiment(taskInverted, 99);
			if (m_triggerContainer->ProcessedTriggerCount() == 0)
			{
				emit sendLogInfo("No Trigger found for this experiment, aborting inverted trialmats generation");
			}
			else
			{
				TrialMatricesProcessor tmp;
				tmp.Process(m_triggerContainer, myeegContainer, taskInverted, freqFolder, m_statOption, m_picOption);
				emit sendLogInfo("Time Trials Matrices generated");
			}
       }
       if(taskBarPlot != nullptr)
       {
           m_triggerContainer->ProcessEventsForExperiment(taskBarPlot, 99);
           if (m_triggerContainer->ProcessedTriggerCount() == 0)
           {
               emit sendLogInfo("No Trigger found for this experiment, aborting Barplot generation");
           }
           else
           {
			   BarplotProcessor bpp;
			   bpp.Process(m_triggerContainer, myeegContainer, taskBarPlot, freqFolder, m_statOption, m_picOption);
			   emit sendLogInfo("Barplots generated");
           }
       }
       emit incrementAdavnce(1);
   }

    //Process Correlation Maps
    if (a.correMaps)
    {
		CorrelationMapsProcessor cmp;
		cmp.Process(myeegContainer, freqFolder, m_PtsFilePath);
		emit sendLogInfo("Correlation map generated");
        emit incrementAdavnce(1);
    }

    //Process Statistical Files
    if(a.statFiles)
    {
        if(taskStatistics != nullptr)
        {
            m_triggerContainer->ProcessEventsForExperiment(taskStatistics, 99);
            if (m_triggerContainer->ProcessedTriggerCount() == 0)
            {
                emit sendLogInfo("No Trigger found for this experiment, aborting trialmats generation");
            }
            else
            {
                StatisticalFilesProcessor sfp;
                sfp.Process(m_triggerContainer, myeegContainer, a.smoothingIDToUse, taskStatistics, freqFolder, m_statOption);
                emit sendLogInfo("Statistical files generated");
            }
        }
        if(taskMaintenanceStatistics != nullptr)
        {
            m_triggerContainer->ProcessEventsForExperiment(taskMaintenanceStatistics, 99);
            if (m_triggerContainer->ProcessedTriggerCount() == 0)
            {
                emit sendLogInfo("No Trigger found for this experiment, aborting trialmats generation");
            }
            else
            {
                StatisticalFilesProcessor sfp;
                sfp.Process(m_triggerContainer, myeegContainer, a.smoothingIDToUse, taskMaintenanceStatistics, freqFolder, m_statOption);
                emit sendLogInfo("Statistical files generated");
            }
        }
        emit incrementAdavnce(1);
    }

    deleteAndNullify1D(m_triggerContainer);
    EEGFormat::Utility::DeleteAndNullify(task);
    EEGFormat::Utility::DeleteAndNullify(taskInverted);
    EEGFormat::Utility::DeleteAndNullify(taskBarPlot);
    EEGFormat::Utility::DeleteAndNullify(taskStatistics);
    EEGFormat::Utility::DeleteAndNullify(taskMaintenanceStatistics);
}

void InsermLibrary::LOCA::CreateEventsFile(FrequencyBandAnalysisOpt analysisOpt, eegContainer* myeegContainer, TriggerContainer* triggerContainer, ProvFile* myprovFile)
{
//    qDebug() << myeegContainer->RootOutputFileFolder().c_str();
//    qDebug() << myeegContainer->RootFileName().c_str();

    vec1<std::string> patientNameSplit = split<std::string>(myeegContainer->RootFileName(), "_");
    std::string sub = patientNameSplit[0];
    std::string task = patientNameSplit[2];
    std::string fileNameBase =  myeegContainer->RootOutputFileFolder() + sub + "_" + task;
    std::string frequencySuffix = "f" + std::to_string(analysisOpt.Band.FMin()) + "f" + std::to_string(analysisOpt.Band.FMax());
    std::string labelName = sub + "_" + task + "_acq-" + frequencySuffix + "ds" +   std::to_string(myeegContainer->DownsamplingFactor());

//    qDebug() << fileNameBase.c_str();
//    qDebug() << frequencySuffix.c_str();
//    qDebug() << labelName.c_str();

	EEGFormat::FileType outputType = analysisOpt.analysisParameters.outputType;
	switch (outputType)
	{
        case EEGFormat::FileType::BrainVision:
        {
            std::string eventFilePath = fileNameBase + ".vmrk";
            std::string downsampledEventsFilePath = myeegContainer->RootOutputFileFolder() + labelName + "_ieeg.vmrk";
            std::vector<Trigger> triggers = triggerContainer->GetTriggerForExperiment(myprovFile, 99);
            std::vector<Trigger> triggersDownsampled = triggerContainer->GetTriggerForExperiment(myprovFile, 99, myeegContainer->DownsamplingFactor());

            //We do not add the datafile indication for the ds.vmrk because we chose not to duplicate said file for each fXX_fYY_dsZ eeg file
            //and thus we can not make it point toward each of the analysis data file.
            //std::string eventDataFilePath = myeegContainer->RootOutputFileFolder() + "/" + myeegContainer->RootFileName() + ".eeg";
            //CreateFile(outputType, eventFilePath, triggers, eventDataFilePath);
            CreateFile(outputType, downsampledEventsFilePath, triggersDownsampled, "");

            RelinkAnalysisFileAnUglyWay(myeegContainer->RootOutputFileFolder(), labelName, frequencySuffix, std::to_string(myeegContainer->DownsamplingFactor()));
            break;
        }
        case EEGFormat::FileType::EuropeanDataFormat:
        {
            throw std::runtime_error("European Data Format file type is not allowed as an output file");
            break;
        }
        default:
        {
            throw std::runtime_error("Output file type not recognized");
            break;
	}
	}
}

void InsermLibrary::LOCA::CreateFile(EEGFormat::FileType outputType, std::string filePath, std::vector<Trigger>& triggers, std::string extraFilePath)
{
	std::vector<EEGFormat::ITrigger> iTriggers(triggers.size());
	for (int i = 0; i < static_cast<int>(iTriggers.size()); i++)
	{
		iTriggers[i] = EEGFormat::ITrigger(triggers[i].MainEvent());
	}

	switch (outputType)
	{
	case EEGFormat::FileType::Micromed:
	{
		throw std::runtime_error("Micromed File type is not allowed as an event output file");
		break;
	}
	case EEGFormat::FileType::Elan:
	{
		EEGFormat::ElanFile::SaveTriggers(filePath, iTriggers);
		break;
	}
	case EEGFormat::FileType::BrainVision:
	{
		EEGFormat::BrainVisionFile::SaveMarkers(filePath, extraFilePath, iTriggers, std::vector<EEGFormat::INote>());
		break;
	}
	case EEGFormat::FileType::EuropeanDataFormat:
	{
		throw std::runtime_error("European Data Format file type is not allowed as an output file");
		break;
	}
	default:
	{
		throw std::runtime_error("Output file type not recognized");
		break;
	}
	}
}

//Since each BrainVision file offers the possiblity to link an event file and for
//each dsX_smX file we want it to point to the same marker file after creating the data
//we need to open each vhdr file and change the path by hand
void InsermLibrary::LOCA::RelinkAnalysisFileAnUglyWay(const std::string& rootPath, const std::string& fileNameBase, const std::string& frequencySuffix, const std::string& downsamplingFactor)
{
	std::string frequencyFolder = fileNameBase + "_" + frequencySuffix;

    //qDebug() << fileNameBase.c_str();
    std::vector<std::string> pathToCheck;
    pathToCheck.push_back(rootPath + frequencySuffix + "/" + fileNameBase + "sm0_ieeg.vhdr");
    pathToCheck.push_back(rootPath + frequencySuffix + "/" + fileNameBase + "sm250_ieeg.vhdr");
    pathToCheck.push_back(rootPath + frequencySuffix + "/" + fileNameBase + "sm500_ieeg.vhdr");
    pathToCheck.push_back(rootPath + frequencySuffix + "/" + fileNameBase + "sm1000_ieeg.vhdr");
    pathToCheck.push_back(rootPath + frequencySuffix + "/" + fileNameBase + "sm2500_ieeg.vhdr");
    pathToCheck.push_back(rootPath + frequencySuffix + "/" + fileNameBase + "sm5000_ieeg.vhdr");

	int PathCount = pathToCheck.size();
	for (int i = 0; i < PathCount; i++)
	{
		if (EEGFormat::Utility::IsValidFile(pathToCheck[i]))
		{
			std::vector<std::string> rawHeader = EEGFormat::Utility::ReadTextFile(pathToCheck[i]);
			int LineCount = rawHeader.size();
			if (LineCount == 0)
				throw std::runtime_error("Error, this file should not be empty");

            auto it = std::find_if(rawHeader.begin(), rawHeader.end(), [&](const std::string& str)
			{
				return str.find("MarkerFile=") != std::string::npos;
			});

			if (it != rawHeader.end())
			{
				int id = std::distance(rawHeader.begin(), it);
                rawHeader[id] = "MarkerFile=../" + fileNameBase + "_ieeg.vmrk";

				std::ofstream markersFile(pathToCheck[i], std::ios::trunc | std::ios::binary);
				if (markersFile.is_open())
				{
					for (int j = 0; j < LineCount; j++)
					{
						markersFile << rawHeader[j] << std::endl;
					}
				}
				markersFile.close();
			}
		}
	}
}

/**************************************************/
/*		Elan Compatibility : Conf File			  */
/**************************************************/
void InsermLibrary::LOCA::CreateConfFile(eegContainer* myeegContainer)
{
	std::string outputConfFilePath = myeegContainer->RootFileFolder() + myeegContainer->RootFileName();
	int electrodesCount = static_cast<int>(myeegContainer->flatElectrodes.size());

	std::ofstream confFile(outputConfFilePath + ".conf", std::ios::out);
	confFile << "nb_channel" << "  " << electrodesCount << std::endl;
	confFile << "sec_per_page" << "  " << 4 << std::endl;
	confFile << "amp_scale_type" << "  " << 1 << std::endl;
	confFile << "amp_scale_val" << "  " << 1 << std::endl;

	confFile << "channel_visibility" << std::endl;
	for (int i = 0; i < electrodesCount; i++)
	{
		confFile << 1 << std::endl;
	}
	/**********************************************************/
	/* We print the bipole Id as long as the next is from the */
	/* same Electrode : A'1, A'2, A'3 ... then when the next  */
	/* one is not from the same Electrode (B'1) we put -1 and */
	/*				we go again until the end				  */
	/**********************************************************/
	confFile << "channel_reference" << std::endl;
	confFile << "-1" << std::endl;

	for (int i = 0; i < myeegContainer->BipoleCount() - 1; i++)
	{
		if (myeegContainer->Bipole(i).second + 1 != myeegContainer->Bipole(i + 1).second)
		{
			//-1
			confFile << myeegContainer->Bipole(i).second << std::endl;
			confFile << "-1" << std::endl;
		}
		else
		{
			confFile << myeegContainer->Bipole(i).second << std::endl;
		}
	}
	confFile.close();
}

//! Create output folder if it does not exist
/*!
  \param myeegContainer contains the path informations of the data to process
  \param currentFreq contains the frequency informations
  \return The path of the created folder
*/
std::string InsermLibrary::LOCA::CreateFrequencyFolder(eegContainer* myeegContainer, FrequencyBand currentFreq)
{
    std::filesystem::path root(myeegContainer->RootFileFolder());
    std::string rootFileFolder = root.parent_path().parent_path().parent_path().parent_path().string() + "/derivatives";
    std::string patientName = myeegContainer->RootFileName();
    vec1<std::string> patientNameSplit = split<std::string>(patientName, "_");
    patientName = patientNameSplit[0];

	std::string fMin = std::to_string(currentFreq.FMin());
	std::string fMax = std::to_string(currentFreq.FMax());
    std::string freqFolder = rootFileFolder + "/" + patientName + "/ieeg/" + "f" + fMin + "f" + fMax + "/";

	if (!QDir(&freqFolder.c_str()[0]).exists())
	{
		emit sendLogInfo(QString::fromStdString("Creating Output Folder for" + fMin + " -> " + fMax + " Hz data"));
		QDir().mkdir(&freqFolder.c_str()[0]);
	}

	return freqFolder;
}

InsermLibrary::ProvFile* InsermLibrary::LOCA::LoadProvForTask(std::string taskName, std::string analysisName)
{
    //m_currentLoca->localizerName()
    std::string taskLabel = analysisName == "" ? taskName : taskName + "_" + analysisName;
    std::string MainProvPath = QCoreApplication::applicationDirPath().toStdString() + "/Resources/Config/Prov/" + taskLabel + ".prov";
    if (EEGFormat::Utility::DoesFileExist(MainProvPath))
    {
        return new ProvFile(MainProvPath);
    }
    else
    {
        return nullptr;
    }
}
