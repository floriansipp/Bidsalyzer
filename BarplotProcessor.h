#ifndef BARPLOTPROCESSOR_H
#define BARPLOTPROCESSOR_H

#include <iostream>
#include <vector>

#include "TriggerContainer.h"
#include "eegContainer.h"
#include "ProvFile.h"
#include "optionsParameters.h"
#include "Stats.h"

namespace InsermLibrary
{
    class BarplotProcessor
    {
    public:
        void Process(TriggerContainer* triggerContainer, eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder, statOption* statOption, picOption* picOption);

    private:
        std::string GetBarplotMapsFolder(std::string freqFolder, ProvFile* myprovFile, statOption* statOption);
        std::string PrepareFolderAndPathsBar(std::string freqFolder, int dsSampFreq);
        std::vector<PVALUECOORD> ProcessKruskallStatistic(vec3<float>& bigData, TriggerContainer* triggerContainer, eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder, statOption* statOption);
    };
}

#endif