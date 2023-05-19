#ifndef TRIALMATRICESPROCESSOR_H
#define TRIALMATRICESPROCESSOR_H

#include <iostream>

#include "TriggerContainer.h"
#include "eegContainer.h"
#include "ProvFile.h"
#include "optionsParameters.h"
#include "Stats.h"

namespace InsermLibrary
{
    class TrialMatricesProcessor
    {
    public:
        void Process(TriggerContainer* triggerContainer, eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder, statOption* statOption, picOption* picOption);

    private:
        std::string GetTrialmatFolder(ProvFile* myprovFile, std::string freqFolder, statOption* statOption);
        std::string PrepareFolderAndPathsTrial(std::string freqFolder, eegContainer* myeegContainer);
        std::vector<PVALUECOORD> ProcessWilcoxonStatistic(vec3<float>& bigData, TriggerContainer* triggerContainer, eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder, statOption* statOption);
    };
}

#endif
