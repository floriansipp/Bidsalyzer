#include "eegContainer.h"
#include "QtCore/qdebug.h"

InsermLibrary::eegContainer::eegContainer(EEGFormat::IFile* file, int downsampFrequency)
{
	fftwf_init_threads();
	fftwf_plan_with_nthreads(5);

	elanFrequencyBand = std::vector<EEGFormat::IFile*>(6);

	m_file = file;
	GetElectrodes(m_file);
	//==
	m_originalSamplingFrequency = m_file->SamplingFrequency();
	m_downsampledFrequency = downsampFrequency;
	m_nbSample = Data().size() > 0 ? Data()[0].size() : 0;
}

InsermLibrary::eegContainer::~eegContainer()
{
	EEGFormat::Utility::DeleteAndNullify(m_file);
	for (int i = 0; i < 6; i++)
	{
		EEGFormat::Utility::DeleteAndNullify(elanFrequencyBand[i]);
	}
	elanFrequencyBand.clear();
	fftwf_cleanup_threads();
}

void InsermLibrary::eegContainer::DeleteElectrodes(std::vector<int> elecToDelete)
{
	if (m_file != nullptr)
	{
		//Data is supposed to be loaded at this point
		//Convert analog to digital => still usefull ? 
		m_file->DeleteElectrodesAndData(elecToDelete);
		//Copy data in vector eeg data
	}
}

void InsermLibrary::eegContainer::GetElectrodes()
{
	if (m_file != nullptr)
	{
		GetElectrodes(m_file);
	}
}

void InsermLibrary::eegContainer::MonoElectrodes()
{
    if (m_bipoles.size() > 0)
        m_bipoles.clear();

    int totalPlot = 0;

    for (int i = 0; i < electrodes.size(); i++)
    {
        for (int j = 0; j < electrodes[i].id.size(); j++)
        {
            m_bipoles.push_back(std::make_pair(totalPlot + j, -1));
        }
        totalPlot += (int)electrodes[i].id.size();
    }
}

void InsermLibrary::eegContainer::BipolarizeElectrodes()
{
	if (m_bipoles.size() > 0)
		m_bipoles.clear();

	int totalPlot = 0;

	for (int i = 0; i < electrodes.size(); i++)
	{
		for (int j = 1; j < electrodes[i].id.size(); j++)
		{
			if ((electrodes[i].id[j] - electrodes[i].id[j - 1]) == 1)
			{
				m_bipoles.push_back(std::make_pair(totalPlot + j, totalPlot + (j - 1)));
			}
		}
		totalPlot += (int)electrodes[i].id.size();
	}
}

void InsermLibrary::eegContainer::SaveFrequencyData(EEGFormat::FileType FileType, const std::vector<int>& frequencyBand)
{
    std::filesystem::path root(m_file->DefaultFilePath());
    qDebug() << root.parent_path().parent_path().parent_path().parent_path().c_str();
    std::string rootFileFolder = root.parent_path().parent_path().parent_path().parent_path().string() + "/derivatives";
    //std::string rootFileFolder = EEGFormat::Utility::GetDirectoryPath(m_file->DefaultFilePath());
    qDebug() << rootFileFolder.c_str();
	std::string patientName = EEGFormat::Utility::GetFileName(m_file->DefaultFilePath(), false);
    vec1<std::string> patientNameSplit = split<std::string>(patientName, "_");
    patientName = patientNameSplit[0];
    qDebug() << patientName.c_str();
    std::string frequencyFolder = "f" + std::to_string(frequencyBand[0]) + "f" + std::to_string(frequencyBand[frequencyBand.size() - 1]);
    qDebug() << frequencyFolder.c_str();
    std::string rootFrequencyFolder = rootFileFolder + "/" + patientName + "/ieeg/" + frequencyFolder + "/";
    qDebug() << rootFrequencyFolder.c_str();

    //TODO : When eeg format does not need boost::filesystem anymore
    //replace functions with those from std::filesystem
    if(!EEGFormat::Utility::IsValidDirectory(rootFrequencyFolder.c_str()))
    {
        std::cout << "Creating freQ FOLDER" << rootFrequencyFolder << std::endl;
		CREATE_DIRECTORY(rootFrequencyFolder.c_str());
    }

    //for bids issues
    patientName = patientNameSplit[0] + "_" + patientNameSplit[2];
    for (int i = 0; i < 6; i++)
	{
		std::string directory = rootFrequencyFolder;
        std::string baseFileName = patientName + "_acq-" + frequencyFolder + "ds" + std::to_string(DownsamplingFactor()) + "sm" + std::to_string((int)m_smoothingMilliSec[i]) + "_ieeg";
		switch (FileType)
		{
			case EEGFormat::FileType::BrainVision:
			{
				std::string header = rootFrequencyFolder + baseFileName + ".vhdr";
				std::string data = rootFrequencyFolder + baseFileName + ".eeg";
			
				EEGFormat::BrainVisionFile* bvFile = new EEGFormat::BrainVisionFile(*elanFrequencyBand[i]);
				bvFile->HeaderFilePath(header);
				bvFile->DataFilePath(data);
				bvFile->MarkersFilePath("");
				bvFile->Save();

				EEGFormat::Utility::DeleteAndNullify(bvFile);
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
}

//Advised order for filePaths : header-data-events-notes
int InsermLibrary::eegContainer::LoadFrequencyData(std::vector<std::string>& filesPath, int smoothingId)
{
	std::string concatenatedFiles = "";
	for (int i = 0; i < filesPath.size(); i++)
	{
		concatenatedFiles += filesPath[i];
		if (i < filesPath.size() - 1)
			concatenatedFiles += ";";
	}

	EEGFormat::Utility::DeleteAndNullify(elanFrequencyBand[smoothingId]);
	elanFrequencyBand[smoothingId] = CreateGenericFile(concatenatedFiles.c_str(), true);
	if (elanFrequencyBand[smoothingId] == nullptr)
		return -1;
	else
		return 0;

}

void InsermLibrary::eegContainer::GetFrequencyBlocData(vec3<float>& outputEegData, int smoothingId, std::vector<Trigger>& triggEeg, int winSam[2])
{
	int TriggerCount = triggEeg.size();
	if (triggEeg[0].SamplingFrequency() != elanFrequencyBand[smoothingId]->SamplingFrequency())
	{
		for (int i = 0; i < TriggerCount; i++)
		{
			triggEeg[i].UpdateFrequency(elanFrequencyBand[smoothingId]->SamplingFrequency());
		}
	}

    std::vector<std::vector<float>> currentData = elanFrequencyBand[smoothingId]->Data(EEGFormat::DataConverterType::Analog);
	for (int i = 0; i < elanFrequencyBand[smoothingId]->ElectrodeCount(); i++)
	{
		for (int j = 0; j < TriggerCount; j++)
		{
			int trigTime = triggEeg[j].MainSample();
			int beginTime = trigTime + winSam[0];

			for (int k = 0; k < (winSam[1] - winSam[0]); k++)
			{
				//to prevent issue in case the first event has been recorded realy quick
				if (beginTime + k < 0)
					outputEegData[i][j][k] = 0;
				else
                    outputEegData[i][j][k] = (currentData[i][beginTime + k] - 100);
			}
		}
	}
}

void InsermLibrary::eegContainer::GetFrequencyBlocDataEvents(vec3<float>& outputEegData, int smoothingId, std::vector<Trigger>& triggEeg, int winSam[2])
{
	int TriggerCount = triggEeg.size();
	if (triggEeg[0].SamplingFrequency() != elanFrequencyBand[smoothingId]->SamplingFrequency())
	{
		for (int i = 0; i < TriggerCount; i++)
		{
			triggEeg[i].UpdateFrequency(elanFrequencyBand[smoothingId]->SamplingFrequency());
		}
	}

	std::vector<std::vector<float>> currentData = elanFrequencyBand[smoothingId]->Data(EEGFormat::DataConverterType::Analog);
	for (int i = 0; i < TriggerCount; i++)
	{
		for (int j = 0; j < winSam[1] - winSam[0]; j++)
		{
			int trigTime = triggEeg[i].MainSample();
			int beginTime = trigTime + winSam[0];

			for (int k = 0; k < elanFrequencyBand[smoothingId]->ElectrodeCount(); k++)
			{
				//to prevent issue in case the first event has been recorded realy quick
				if (beginTime + j < 0)
					outputEegData[i][k][j] = 0;
				else
					outputEegData[i][k][j] = (currentData[k][beginTime + j] - 100);
			}
		}
	}
}

//=== Private :
void InsermLibrary::eegContainer::GetElectrodes(EEGFormat::IFile* edf)
{
	if (electrodes.size() > 0)
		electrodes.clear();

	if (flatElectrodes.size() > 0)
		flatElectrodes.clear();

    std::string elecNameStringTemp = "%#";
	for (int i = 0; i < edf->Electrodes().size(); i++)
	{
        std::string result = "";
		int resId = -1;

        int goodId = GetIndexFromElectrodeLabel(edf->Electrodes()[i]->Label());

		if (goodId != -1)
		{
			result = edf->Electrodes()[i]->Label().substr(0, goodId);
			resId = stoi(edf->Electrodes()[i]->Label().substr(goodId, edf->Electrodes()[i]->Label().size()));
		}
		else
		{
			result = edf->Electrodes()[i]->Label();
		}

		if (result.find(elecNameStringTemp) != std::string::npos && (result.length() == elecNameStringTemp.length()))
		{
			/*cout << result << " et " << resId << endl;*/
			electrodes[electrodes.size() - 1].id.push_back(resId);
			electrodes[electrodes.size() - 1].idOrigFile.push_back(i);
            flatElectrodes.push_back(result + std::to_string(resId));
		}
		else
		{
			//cout << "[=====]"<< endl;
			//cout << result << " et " << resId << endl;
			elecNameStringTemp = result;
			electrodes.push_back(elecContainer());
			electrodes[electrodes.size() - 1].label = result;
			electrodes[electrodes.size() - 1].id.push_back(resId);
			electrodes[electrodes.size() - 1].idOrigFile.push_back(i);
            flatElectrodes.push_back(result + std::to_string(resId));
		}
	}
}

int InsermLibrary::eegContainer::GetIndexFromElectrodeLabel(std::string myString)
{
	for (int j = 0; j < myString.size(); j++)
	{
		if (isdigit(myString[j]) && myString[j] != 0)
		{
			return j;
		}
	}
	return -1;
}
