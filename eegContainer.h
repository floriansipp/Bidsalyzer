#ifndef _EEGCONTAINER_H
#define _EEGCONTAINER_H

#include "../../EEGFormat/EEGFormat/IFile.h"
#include "../../EEGFormat/EEGFormat/ElanFile.h"
#include "../../EEGFormat/EEGFormat/BrainVisionFile.h"
#include "../../EEGFormat/EEGFormat/Utility.h"
#include "../../EEGFormat/EEGFormat/Wrapper.h"
#include "eegContainerParameters.h"
#include "DataContainer.h"
#include "Trigger.h"
#include "Utility.h"

#include <iostream>
#include <vector>
#include <thread>
#include <mutex> 

namespace InsermLibrary
{
	class eegContainer
	{
	public:
        eegContainer(EEGFormat::IFile* file, int downsampFrequency);
		~eegContainer();		

		//===[ Getter / Setter ]===
		inline std::string RootFileFolder()
		{
			return EEGFormat::Utility::GetDirectoryPath(m_file->DefaultFilePath());
		}
		inline std::string RootFileName(bool withExtension = false)
		{
			return EEGFormat::Utility::GetFileName(m_file->DefaultFilePath(), withExtension);
		}
		inline int SamplingFrequency()
		{
			return m_originalSamplingFrequency;
		}
		inline int DownsampledFrequency()
		{
			return m_downsampledFrequency;
		}
		inline int DownsamplingFactor()
		{
			return m_originalSamplingFrequency / m_downsampledFrequency;
		}
		inline int NbSample()
		{
			return Data().size() > 0 ? Data()[0].size() : 0;
		}
		inline int TriggerCount()
		{
			return m_file->TriggerCount();
		}
		inline std::vector<EEGFormat::ITrigger> Triggers()
		{
			int TriggerCount = m_file->TriggerCount();
			std::vector<EEGFormat::ITrigger> triggers(TriggerCount);
			for (int i = 0; i < TriggerCount; i++)
			{
				triggers[i] = EEGFormat::ITrigger(*m_file->Trigger(i));
			}
			return triggers;
		}
		inline EEGFormat::IElectrode* Electrode(const int& i) const
		{
			int ElectrodeCount = m_file->ElectrodeCount();
			if (i >= 0 && i < ElectrodeCount)
			{
				return m_file->Electrode(i);
			}
			return nullptr;
		}
		inline int BipoleCount()
		{
			return m_bipoles.size();
		}
		//First is positiv, second is negativ Electrode ID
		inline std::pair<int,int> Bipole(const int& i)
		{
            if (i >= 0 && i < static_cast<int>(m_bipoles.size()))
			{
				return m_bipoles[i];
			}
			return std::pair<int, int>(-1, -1);
		}
		inline std::string BipolePositivLabel(const int& i)
		{
            if (i >= 0 && i < static_cast<int>(m_bipoles.size()))
			{
				return flatElectrodes[m_bipoles[i].first];
			}
			return "";
		}
		inline std::string BipoleNegativLabel(const int& i)
		{
            if (i >= 0 && i < static_cast<int>(m_bipoles.size()))
			{
				return flatElectrodes[m_bipoles[i].second];
			}
			return "";
		}
		inline std::vector<std::vector<float>>& Data() { return m_file->Data(EEGFormat::DataConverterType::Digital); }
		inline const std::vector<std::vector<float>>& Data() const { return m_file->Data(EEGFormat::DataConverterType::Digital); }

		//===[ Data Modification ]===
        void DeleteElectrodes(std::vector<int> elecToDelete);
		void GetElectrodes();
        void MonoElectrodes();
		void BipolarizeElectrodes();
		void SaveFrequencyData(EEGFormat::FileType FileType, const std::vector<int>& frequencyBand);
		int LoadFrequencyData(std::vector<std::string>& filesPath, int smoothingId);

		//===[ Read / Get Data ]===
		void GetFrequencyBlocData(vec3<float>& outputEegData, int smoothingId, std::vector<Trigger>& triggEeg, int winSam[2]);
		void GetFrequencyBlocDataEvents(vec3<float>& outputEegData, int smoothingId, std::vector<Trigger>& triggEeg, int winSam[2]);

	private:
		void GetElectrodes(EEGFormat::IFile* edf);
        int GetIndexFromElectrodeLabel(std::string myString);

	public :
		std::vector<EEGFormat::IFile*> elanFrequencyBand;
        std::vector<elecContainer> electrodes;
        std::vector<std::string> flatElectrodes;

	private:
		int m_originalSamplingFrequency = 0;
		int m_downsampledFrequency = 0;
		int m_nbSample = 0; //Original size of one channel (no downsamp)
		std::vector<std::pair<int, int>> m_bipoles;
		float m_smoothingMilliSec[6] = { 0, 250, 500, 1000, 2500, 5000 };
		EEGFormat::IFile* m_file = nullptr;
		std::mutex m_mtx;
	};
}
#endif
