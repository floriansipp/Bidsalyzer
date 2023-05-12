#include "TriggerContainer.h"

InsermLibrary::TriggerContainer::TriggerContainer(std::vector<EEGFormat::ITrigger>& triggers, int samplingFrequnecy)
{
	m_originalSamplingFrequency = samplingFrequnecy;
	int TriggersCount = triggers.size();
	for (int i = 0; i < TriggersCount; i++)
	{
		m_originalTriggers.push_back(EEGFormat::ITrigger(triggers[i]));
	}
}

InsermLibrary::TriggerContainer::~TriggerContainer()
{

}

// Return a list of trigger for the wanted experiment
// Use flagCode = -1 to get the full trigger list
std::vector<InsermLibrary::Trigger> InsermLibrary::TriggerContainer::GetTriggerForExperiment(ProvFile *myprovFile, int flagCode, int downSamplingFactor)
{
	std::vector<Trigger> triggers = GetTriggerList(flagCode, downSamplingFactor);
	if (myprovFile != nullptr && triggers.size() != 0)
	{
        std::string chgFilePath = myprovFile->FilePath();
        int pos = chgFilePath.find(".prov");
        if( pos != std::string::npos )
            chgFilePath.replace(pos, 5, ".chg");

        if(std::filesystem::exists(chgFilePath))
		{
            RenameTriggersForExperiment(myprovFile, chgFilePath, triggers);
		}
	}
	return triggers;
}

std::vector<InsermLibrary::Trigger> InsermLibrary::TriggerContainer::GetTriggerList(int flagCode, int downSamplingFactor)
{
	int beginvalue = 0;
	if (flagCode != -1)
		beginvalue = FindFirstIndexAfter(flagCode);
	std::vector<InsermLibrary::Trigger> triggers;
	if (m_originalTriggers.size() > 0)
	{
		for (int i = beginvalue; i < m_originalTriggers.size(); i++)
		{
			Trigger currentTrigger(m_originalTriggers[i], m_originalSamplingFrequency);
			currentTrigger.UpdateFrequency(m_originalSamplingFrequency / downSamplingFactor);
			triggers.push_back(currentTrigger);
		}
	}
	return triggers;
}

void InsermLibrary::TriggerContainer::ProcessEventsForExperiment(ProvFile *myprovFile, int flagCode, int downSaplingFactor)
{
	m_processedTriggers = GetTriggerList(flagCode, downSaplingFactor);
	if (m_processedTriggers.size() == 0)
		return;

    std::string chgFilePath = myprovFile->FilePath();
    int pos = chgFilePath.find(".prov");
    if( pos != std::string::npos )
        chgFilePath.replace(pos, 5, ".chg");

    if(std::filesystem::exists(chgFilePath))
	{
        RenameTriggersForExperiment(myprovFile, chgFilePath, m_processedTriggers);
	}
	PairStimulationWithResponses(m_processedTriggers, myprovFile);
	DeleteTriggerNotInExperiment(m_processedTriggers, myprovFile);

	//At this point, if there is a secondary code, we need to check that they have been paired,
	//otherwise we delete them and only keep those that have been paired
    DeleteTriggerNotPaired(m_processedTriggers, myprovFile);

	if (m_processedTriggers.size() == 0)
		return;

	m_codeAndTrials = SortTrialsForExperiment(m_processedTriggers, myprovFile);
	if (myprovFile->FilePath().find("INVERTED") != std::string::npos)
	{
		std::string invertedFilePath = myprovFile->FilePath();
		int pos = invertedFilePath.find(".prov");
		if (pos != std::string::npos)
			invertedFilePath.replace(pos, 5, ".txt");

		if (std::filesystem::exists(invertedFilePath))
		{
			std::stringstream buffer;						
			std::ifstream provFile(invertedFilePath, std::ios::binary);
			if (provFile)	
			{																
				buffer << provFile.rdbuf();						
				provFile.close();			
			}			
			else	
			{
				std::cout << " Error opening Prov File @ " << invertedFilePath << std::endl;
			}
			
			std::vector<std::string>splitInvertWin = split<std::string>(buffer.str(), "|");

			std::vector<std::string>splitInvertEpochWin = split<std::string>(splitInvertWin[0], ":");
			int winStart = atoi(&splitInvertEpochWin[0][0]);
			int winEnd = atoi(&splitInvertEpochWin[1][0]);
			int* newWindow = new int[2]{ winStart, winEnd };
			std::vector<std::string>splitInvertBaseLineWin = split<std::string>(splitInvertWin[1], ":");
			int baselineStart = atoi(&splitInvertBaseLineWin[0][0]);
			int baselineEnd = atoi(&splitInvertBaseLineWin[1][0]);
			int* newBaseline = new int[2]{ baselineStart, baselineEnd };

			SwapStimulationsAndResponses(myprovFile, newWindow, newBaseline);

			delete[] newWindow;
			delete[] newBaseline;
		}
	}
}

void InsermLibrary::TriggerContainer::RenameTriggersForExperiment(ProvFile *myprovFile, std::string chgFilePath, std::vector<Trigger>& triggers)
{
    std::vector<std::string> rawChangeCodeData = EEGFormat::Utility::ReadTextFile(chgFilePath);

	std::vector<std::vector<std::pair<int, int>>> oldCodes;
	std::vector<std::pair<int, int>> newCodes;
	for (int i = 0; i < rawChangeCodeData.size(); i++)
	{
        std::vector<std::string> currentRawCodes = EEGFormat::Utility::Split<std::string>(rawChangeCodeData[i], "+=");
		std::pair<int, int> currentOldCode = std::make_pair(atoi(&(currentRawCodes[0])[0]), atoi(&(currentRawCodes[1])[0]));
		std::pair<int, int> currenNewCode = std::make_pair(atoi(&(currentRawCodes[2])[0]), atoi(&(currentRawCodes[3])[0]));

		auto it = std::find(newCodes.begin(), newCodes.end(), currenNewCode);
		if (it != newCodes.end()) //if the pair already exists
		{
			int indexOfPair = distance(newCodes.begin(), it);
			oldCodes[indexOfPair].push_back(currentOldCode);
		}
		else
		{
			newCodes.push_back(currenNewCode);
			//==
			std::vector<std::pair<int, int>> newVector;
			newVector.push_back(currentOldCode);
			oldCodes.push_back(newVector);
		}
	}

	int idVisuBloc = 0;
	int TriggerCount = triggers.size();
	for (int k = 0; k < TriggerCount; k++)
	{
        int idVisuBloc = -1;
		int idMain = -1;
		int idSec = -1;

        for (int l = 0; l < oldCodes.size(); l++)
		{
			for (int m = 0; m < oldCodes[l].size(); m++)
			{
				if (triggers[k].MainEvent().Code() == oldCodes[l][m].first)
				{
					idMain = k;
                    for (int n = 0; n < myprovFile->Blocs().size(); n++)
                    {
                        Event mainEvent = myprovFile->Blocs()[n].MainSubBloc().MainEvent();
                        if (newCodes[l].first == mainEvent.Codes()[0])
							idVisuBloc = n;
					}
				}
			}
		}

		//TODO : add a function that check there is not some triggerd with a different
		//sampling frequnecy, otherwise throw exception
		int memId = 0;
		int triggersSamplingFrequency = triggers[0].SamplingFrequency();
		if (idMain != -1)
        {
            int winSamMin = round((triggersSamplingFrequency * myprovFile->Blocs()[idVisuBloc].MainSubBloc().MainWindow().Start()) / 1000);
            int winSamMax = round((triggersSamplingFrequency * myprovFile->Blocs()[idVisuBloc].MainSubBloc().MainWindow().End()) / 1000);

			int CurrentTriggerId = k + 1;
			while (idSec == -1 && CurrentTriggerId < TriggerCount - 1)
			{
				for (int l = 0; l < oldCodes.size(); l++)
				{
					for (int m = 0; m < oldCodes[l].size(); m++)
					{
						if (triggers[idMain].MainCode() == oldCodes[l][m].first && triggers[CurrentTriggerId].MainCode() == oldCodes[l][m].second)
						{
							idSec = CurrentTriggerId;
							memId = l;
							break;
						}
						else if (triggers[CurrentTriggerId].MainCode() == oldCodes[l][m].first && idSec == -1)
						{
							idMain = CurrentTriggerId;
							memId = l;
						}
					}
				}
				CurrentTriggerId++;
			}

			if (idMain != -1 && idSec != -1)
			{
				int winMax = triggers[idMain].MainSample() + winSamMax;
				int winMin = triggers[idMain].MainSample() - abs(winSamMin);

				bool isInWindow = (triggers[idSec].MainSample() < winMax) && (triggers[idSec].MainSample() > winMin);
				//bool specialLoca = (currentLoca->localizerName() == "MARA" ||
				//	currentLoca->localizerName() == "MARM" ||
				//	currentLoca->localizerName() == "MARD");
				if (isInWindow /*|| specialLoca*/)
				{
					triggers[idMain].MainCode(newCodes[memId].first);
					triggers[idSec].MainCode(newCodes[memId].second);
				}
			}
		}
	}
}

void InsermLibrary::TriggerContainer::SwapStimulationsAndResponses(ProvFile *myprovFile, int* newWindow, int* newBaseline)
{
	int TriggerCount = m_processedTriggers.size();
	for (int i = 0; i < TriggerCount; i++)
	{
		m_processedTriggers[i].SwapStimulationAndResponse();
	}

	Window newWin(newWindow[0], newWindow[1]);
	Window newBl(newBaseline[0], newBaseline[1]);

	//This is the new window to visualize data
	for (int i = 0; i < myprovFile->Blocs().size(); i++)
	{
		myprovFile->Blocs()[i].MainSubBloc().MainWindow(newWin);
		myprovFile->Blocs()[i].MainSubBloc().Baseline(newBl);
	}
}

void InsermLibrary::TriggerContainer::PairStimulationWithResponses(std::vector<Trigger>& triggers, ProvFile *myprovFile)
{
    std::vector<int> mainEventsCode = GetMainCodesFromProtocol(myprovFile);
    std::vector<std::vector<int>> respEventsCode = GetSecondaryCodesFromProtocol(myprovFile);

	int idVisuBloc = 0;
	int TriggerCount = triggers.size();
	for (int k = 0; k < TriggerCount; k++)
	{
		int idVisuBloc = -1;
		int idMain = -1;
		int idSec = -1;

		for (int l = 0; l < mainEventsCode.size(); l++)
		{
			if (triggers[k].MainEvent().Code() == mainEventsCode[l])
			{
				idMain = k;
                for (int m = 0; m < myprovFile->Blocs().size(); m++)
				{
                    if (mainEventsCode[l] == myprovFile->Blocs()[m].MainSubBloc().MainEvent().Codes()[0])
						idVisuBloc = m;
				}
			}
		}

		//TODO : add a function that check there is not some triggerd with a different
		//sampling frequnecy, otherwise throw exception
		int triggersSamplingFrequency = triggers[0].SamplingFrequency();
		if (idMain != -1)
		{
            int winSamMin = round((triggersSamplingFrequency * myprovFile->Blocs()[idVisuBloc].MainSubBloc().MainWindow().Start()) / 1000);
            int winSamMax = round((triggersSamplingFrequency * myprovFile->Blocs()[idVisuBloc].MainSubBloc().MainWindow().End()) / 1000);

			int CurrentTriggerId = k + 1;
			while (idSec == -1 && CurrentTriggerId < TriggerCount - 1)
			{
				for (int l = 0; l < mainEventsCode.size(); l++)
				{
					for (int m = 0; m < respEventsCode[l].size(); m++)
					{
						if (triggers[idMain].MainCode() == mainEventsCode[l] && triggers[CurrentTriggerId].MainCode() == respEventsCode[l][m])
						{
							idSec = CurrentTriggerId;
						}
						else if (triggers[CurrentTriggerId].MainCode() == mainEventsCode[l] && idSec == -1)
						{
							idMain = CurrentTriggerId;
						}
					}
				}
				CurrentTriggerId++;
			}

			if (idMain != -1 && idSec != -1)
			{
				int winMax = triggers[idMain].MainSample() + winSamMax;
				int winMin = triggers[idMain].MainSample() - abs(winSamMin);

				bool isInWindow = (triggers[idSec].MainSample() < winMax) && (triggers[idSec].MainSample() > winMin);
				bool specialLoca = false;
				/*bool specialLoca = (currentLoca->localizerName() == "MARA" ||
					currentLoca->localizerName() == "MARM" ||
					currentLoca->localizerName() == "MARD");*/

				if (isInWindow)
				{
					triggers[idMain].Response(triggers[idSec].MainEvent());
				}
				else
				{
					//if it is out of the window but one of the special Loca (MARA, MARM or MARD we want to mark a "fake" reponse 
					//at the maximum window length to keep track of those trials
					if (specialLoca)
					{
						triggers[idMain].Response(triggers[idSec].MainCode(), winMax);
					}
				}
			}
		}
	}
}

//Delete code that are not defined in prov file 
void InsermLibrary::TriggerContainer::DeleteTriggerNotInExperiment(std::vector<Trigger>& triggers, ProvFile *myprovFile)
{
	int TriggerCount = triggers.size();

	std::vector<int> IdToKeep, IdToDelete;
	for (int i = 0; i < TriggerCount; i++)
	{
		bool KeepCurrentTrigger = false;
        for (int j = 0; j < myprovFile->Blocs().size(); j++)
		{
            for (int k = 0; k < myprovFile->Blocs()[j].MainSubBloc().MainEvent().Codes().size(); k++)
			{
                if (triggers[i].MainCode() == myprovFile->Blocs()[j].MainSubBloc().MainEvent().Codes()[k])
				{
					KeepCurrentTrigger = true;
					//break here ? 
				}
			}
		}

		if (KeepCurrentTrigger)
			IdToKeep.push_back(i);
		else
			IdToDelete.push_back(i);
	}

	for (int i = IdToDelete.size() - 1; i >= 0; i--)
		triggers.erase(triggers.begin() + IdToDelete[i]);
}

void InsermLibrary::TriggerContainer::DeleteTriggerNotPaired(std::vector<Trigger>& triggers)
{
	int TriggerCount = triggers.size();
	std::vector<int> IdToDelete;
	for (int i = 0; i < TriggerCount; i++)
	{
		if (triggers[i].ResponseCode() == -1)
		{
			IdToDelete.push_back(i);
		}
	}

	int ToDeleteCount = IdToDelete.size() - 1;
	for (int i = ToDeleteCount; i >= 0; i--)
		triggers.erase(triggers.begin() + IdToDelete[i]);
}

void InsermLibrary::TriggerContainer::DeleteTriggerNotPaired(std::vector<Trigger>& triggers, ProvFile *myprovFile)
{
    for(int i = 0; i < myprovFile->Blocs().size(); i++)
    {
        for(int j = 0; j < myprovFile->Blocs()[i].SubBlocs().size(); j++)
        {
            std::vector<int> IdToDelete;
            SubBloc subbloc = myprovFile->Blocs()[i].SubBlocs()[j];
            if(subbloc.Type() == MainSecondaryEnum::Main)
            {
                int mainCode = subbloc.MainEvent().Codes()[0];
                if(subbloc.SecondaryEvents().size() != 0) //if the blocs has secondary codes but we find codes without response we want to delete them
                {
                    std::vector<int> secondaryCodes = subbloc.SecondaryEvents()[0].Codes();
                    for (int k = 0; k < triggers.size(); k++)
                    {
                        if (triggers[k].MainCode() == mainCode && triggers[k].ResponseCode() == -1)
                        {
                            IdToDelete.push_back(k);
                        }
                    }

                    int ToDeleteCount = IdToDelete.size() - 1;
                    for (int k = ToDeleteCount; k >= 0; k--)
                        triggers.erase(triggers.begin() + IdToDelete[k]);
                }
            }
        }
    }
}

std::vector<std::tuple<int, int, int>> InsermLibrary::TriggerContainer::SortTrialsForExperiment(std::vector<Trigger>& triggers, ProvFile *myprovFile)
{
    std::vector<std::tuple<int, int, int>> codeAndTrials;
    std::vector<Trigger> finalArray;

	for(int i = 0; i < myprovFile->Blocs().size(); i++)
    {
        std::string blocSort = myprovFile->Blocs()[i].Sort();
        std::vector<std::string> blocSortSplitted = split<std::string>(blocSort, ";");

        std::vector<std::tuple<int, int, int>> begAndEnd;
        std::vector<Trigger> sortedArray;
        for(int j = blocSortSplitted.size() - 1; j >= 0; j--)
        {
           std::vector<std::string> blocSortSubAction = split<std::string>(blocSortSplitted[j], "_");
           if(blocSortSubAction.size() >= 3)
           {
               std::string subBlocName = blocSortSubAction[0];
               std::string eventName = "";
               for(int k = 1; k < blocSortSubAction.size() - 1; k++)
               {
                   eventName = eventName + blocSortSubAction[k];
                   if(k + 1 < blocSortSubAction.size() - 1)
                   {
                       eventName = eventName  + "_";
                   }
               }
               std::string sortType = blocSortSubAction[blocSortSubAction.size() - 1];

               if (sortType == "LATENCY")
               {
                   if(sortedArray.size() == 0)
                   {
                       std::vector<int> codes = myprovFile->Blocs()[i].MainSubBloc().MainEvent().Codes();
                       for(int k = 0; k < codes.size(); k++)
                       {
                           std::copy_if(triggers.begin(), triggers.end(), std::back_inserter(sortedArray), [&](Trigger trigger) { return trigger.MainCode() == codes[k]; });
                       }
                       std::sort(sortedArray.begin(), sortedArray.end(),
                           [](Trigger m, Trigger n)-> bool
                           {
                               return m.ReactionTimeInMilliSeconds() < n.ReactionTimeInMilliSeconds();
                           });
                   }
                   else
                   {
                       for(int k = 0; k < begAndEnd.size();k++)
                       {
                           int beg = std::get<1>(begAndEnd[k]);
                           int end = std::get<2>(begAndEnd[k]);
                           std::sort(sortedArray.begin() + beg, sortedArray.begin() + end,
                               [](Trigger m, Trigger n)-> bool
                               {
                                   return m.ReactionTimeInMilliSeconds() < n.ReactionTimeInMilliSeconds();
                               });
                       }
                   }
               }
               else if (sortType == "CODE")
               {
                   std::vector<int> codes = myprovFile->Blocs()[i].MainSubBloc().MainEvent().Codes();
                   for(int k = 0; k < codes.size(); k++)
                   {
                       int beg = sortedArray.size();
                       std::copy_if(triggers.begin(), triggers.end(), std::back_inserter(sortedArray), [&](Trigger trigger) { return trigger.MainCode() == codes[k]; });
                       int end = sortedArray.size() - 1;
                       begAndEnd.push_back(std::make_tuple(codes[k], beg, end));
                   }
               }
           }
        }
        int beg = finalArray.size();
        finalArray.insert(finalArray.end(), sortedArray.begin(), sortedArray.end());
        int end = finalArray.size() - 1;
        int code = myprovFile->Blocs()[i].MainSubBloc().MainEvent().Codes()[0];
        codeAndTrials.push_back(std::make_tuple(code, beg, end));
    }

    triggers = std::vector<Trigger>(finalArray);

    return codeAndTrials;
}

int InsermLibrary::TriggerContainer::FindFirstIndexAfter(int flagCode)
{
	int beginValue = 0;
	std::vector<int> indexBegin = FindIndexes(flagCode);
	if (indexBegin.size() > 0)
		beginValue = indexBegin[indexBegin.size() - 1] + 1;
	return beginValue;
}

std::vector<int> InsermLibrary::TriggerContainer::FindIndexes(int searchValue)
{
	std::vector<int> indexes;
	for (int i = 0; i < m_originalTriggers.size(); i++)
	{
		if (m_originalTriggers[i].Code() == searchValue)
		{
			indexes.push_back(i);
		}
	}
	return indexes;
}

std::vector<int> InsermLibrary::TriggerContainer::GetMainCodesFromProtocol(ProvFile *myprovFile)
{
    std::vector<int> mainCodes;
    std::vector<Bloc> blocs = myprovFile->Blocs();
    for (int m = 0; m < blocs.size(); m++)
    {
        if (find(mainCodes.begin(), mainCodes.end(), blocs[m].MainSubBloc().MainEvent().Codes()[0]) == mainCodes.end())
        {
            mainCodes.push_back(blocs[m].MainSubBloc().MainEvent().Codes()[0]);
        }
    }
    return mainCodes;
}

std::vector<std::vector<int>> InsermLibrary::TriggerContainer::GetSecondaryCodesFromProtocol(ProvFile *myprovFile)
{
    std::vector<std::vector<int>> responseCodes;
    std::vector<Bloc> blocs = myprovFile->Blocs();
    for (int m = 0; m < blocs.size(); m++)
    {
        std::vector<int> blocCodes;
        for (int j = 0; j < blocs[m].MainSubBloc().SecondaryEvents().size(); j++)
        {
            for (int n = 0; n < blocs[m].MainSubBloc().SecondaryEvents()[j].Codes().size(); n++)
            {
                int code = blocs[m].MainSubBloc().SecondaryEvents()[j].Codes()[n];
                if (find(blocCodes.begin(), blocCodes.end(), code) == blocCodes.end())
                {
                    blocCodes.push_back(code);
                }
            }
        }
        responseCodes.push_back(blocCodes);
    }
    return responseCodes;
}
