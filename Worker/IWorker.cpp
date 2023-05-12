#include "IWorker.h"

IWorker::IWorker()
{
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");
}

IWorker::~IWorker()
{
    deleteAndNullify1D(m_Loca);
}

void IWorker::SetExternalParameters(std::vector<int> IndexToDelete, std::vector<std::string> CorrectedLabels, int ElecOperation)
{
    m_IndexToDelete = std::vector<int>(IndexToDelete);
    m_CorrectedLabels = std::vector<std::string>(CorrectedLabels);
    m_ElectrodeOperation = ElecOperation;
}

std::vector<std::string> IWorker::ExtractElectrodeListFromFile(std::string currentFilePath)
{
    emit sendLogInfo(QString::fromStdString("  => Extracting Electrodes from : " + currentFilePath));
    EEGFormat::IFile* file = CreateGenericFile(currentFilePath.c_str(), false);
    std::vector<std::string> ElectrodesList = std::vector<std::string>(file->ElectrodeCount());
    for(size_t i = 0; i < ElectrodesList.size(); i++)
    {
        //===[ Tips ]===
        //Use the char* value for the label so that the string is terminated
        //and does not keep the extra allocated char as white spaces
        ElectrodesList[i] = std::string(file->Electrode(i)->Label().c_str());
    }
    DeleteGenericFile(file);
    return ElectrodesList;
}

std::string IWorker::GetCurrentTime()
{
    std::stringstream TimeDisp;
    std::time_t t = std::time(nullptr);
    TimeDisp << std::put_time(std::localtime(&t), "%c");
    return TimeDisp.str();
}

InsermLibrary::eegContainer* IWorker::GetEegContainer(std::string currentFilePath, bool shouldExtractData)
{
    emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFilePath));
    EEGFormat::IFile* file = CreateGenericFile(currentFilePath.c_str(), shouldExtractData);
    CorrectElectrodeLabels(file);
    return new InsermLibrary::eegContainer(file, 64);
}

void IWorker::CorrectElectrodeLabels(EEGFormat::IFile* file)
{
    if(m_CorrectedLabels.size() == 0)
        return;

    std::vector<EEGFormat::IElectrode*> ElectrodeList = file->Electrodes();
    size_t ElectrodeCount = ElectrodeList.size();
    if(m_CorrectedLabels.size() != ElectrodeList.size())
    {
        emit sendLogInfo("There is not the same number of electrode to correct than in the file, aborting label correction");
        return;
    }

    for(size_t i = 0; i < ElectrodeCount; i++)
    {
        if (m_CorrectedLabels[i].compare("BAD LABELING") == 0) 
            continue;
        ElectrodeList[i]->Label(m_CorrectedLabels[i]);
    }

    file->Electrodes(ElectrodeList);
    emit sendLogInfo("Electrodes label have been corrected");
}
