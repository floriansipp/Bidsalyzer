#ifndef BIDS_SUBJECT_WORKER_H
#define BIDS_SUBJECT_WORKER_H

#include <QCoreApplication>
#include <iostream>
#include <ctime>
#include "IWorker.h"
#include "Data/BidsSubject.h"
#include "FrequencyBandAnalysisOpt.h"
#include "optionsParameters.h" //pour stat et pic options, a separer

class BidsSubjectWorker : public IWorker
{
	Q_OBJECT

public:
    BidsSubjectWorker(BidsSubject subject, std::vector<InsermLibrary::FrequencyBandAnalysisOpt>& FrequencyBands, InsermLibrary::statOption statOption, InsermLibrary::picOption picOption, std::string ptsFilePath);
    ~BidsSubjectWorker();
	void Process();
    void ExtractElectrodeList();

private:
    InsermLibrary::eegContainer* ExtractData(ExperimentFolder currentLoca, bool extractOriginalData);

private:
    BidsSubject* m_Patient = nullptr;
    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> m_FrequencyBands;
    std::vector<InsermLibrary::FileType> m_filePriority;
};

#endif
