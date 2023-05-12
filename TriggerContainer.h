#ifndef _TRIGGERCONTAINER_H
#define _TRIGGERCONTAINER_H

#include "ITrigger.h"
#include "Trigger.h"
#include "ProvFile.h"
#include "Utility.h"

#include <iostream>
#include <algorithm>
#include <tuple>
#include <filesystem>

namespace InsermLibrary
{
	class TriggerContainer
	{
	public:
		enum SortingChoice { Code = 'C', Latency = 'L' };

	public:
		TriggerContainer(std::vector<EEGFormat::ITrigger>& triggers, int samplingFrequnecy);
		~TriggerContainer();
		inline int ProcessedTriggerCount()
		{
			return m_processedTriggers.size();
		}
		inline std::vector<Trigger>& ProcessedTriggers() { return m_processedTriggers; }
		inline const std::vector<Trigger>& ProcessedTriggers() const { return m_processedTriggers; }
		inline const std::vector<std::tuple<int, int, int>>& CodeAndTrialsIndexes() const { return m_codeAndTrials; }
        std::vector<Trigger> GetTriggerForExperiment(ProvFile *myprovFile, int flagCode = 99, int downSamplingFactor = 1);
        void ProcessEventsForExperiment(ProvFile *myprovFile, int flagCode = 99, int downSaplingFactor = 1);
		void SwapStimulationsAndResponses(ProvFile* myprovFile, int* newWindow, int* newBaseline);
	
	private:		
		int FindFirstIndexAfter(int flagCode = 99);
		std::vector<int> FindIndexes(int searchValue);
		std::vector<Trigger> GetTriggerList(int flagCode, int downSamplingFactor = 1);
        void PairStimulationWithResponses(std::vector<Trigger>& triggers, ProvFile *myprovFile);
        void DeleteTriggerNotInExperiment(std::vector<Trigger>& triggers, ProvFile *myprovFile);
		void DeleteTriggerNotPaired(std::vector<Trigger>& triggers);
        void DeleteTriggerNotPaired(std::vector<Trigger>& triggers, ProvFile *myprovFile);
        std::vector<std::tuple<int, int, int>> SortTrialsForExperiment(std::vector<Trigger>& triggers, ProvFile *myprovFile);
        void RenameTriggersForExperiment(ProvFile *myprovFile, std::string chgFilePath, std::vector<Trigger>& triggers);
        std::vector<int> GetMainCodesFromProtocol(ProvFile *myprovFile);
        std::vector<std::vector<int>> GetSecondaryCodesFromProtocol(ProvFile *myprovFile);

	private:
		std::vector<std::tuple<int, int, int>> m_codeAndTrials;
		std::vector<Trigger> m_processedTriggers;
		int m_originalSamplingFrequency = 0;
		std::vector<EEGFormat::ITrigger> m_originalTriggers;
	};
}

#endif //_TRIGGERCONTAINER_H