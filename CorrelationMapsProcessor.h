#ifndef CORRELATIONMAPSPROCESSOR_H
#define CORRELATIONMAPSPROCESSOR_H

#include <iostream>

#include "TriggerContainer.h"
#include "eegContainer.h"
#include "ProvFile.h"
#include "optionsParameters.h"
#include "Stats.h"

namespace InsermLibrary
{
    class CorrelationMapsProcessor
    {
    public:
        void Process(eegContainer* myeegContainer, std::string freqFolder, std::string ptsFilePath);

    private:
        std::string DefineMapPath(std::string freqFolder, int dsSampFreq, int windowSizeInSec);
        std::vector<int> DefineCorrelationWindowsCenter(int halfWindowSizeInSample, int fileSizeInSample);
        std::vector<std::vector<float>> ComputeElectrodesDistances(eegContainer* myeegContainer);
        std::vector<std::vector<float>> ComputeElectrodesDistancesFromPts(eegContainer* myeegContainer, std::string ptsFilePath);
        float ComputeSurrogate(int electrodeCount, int triggerCount, int surrogateCount, vec2<float> distances, vec3<float> eegData);
        void DrawCorrelationCircle(QPainter* painterChanel, eegContainer* myeegContainer, int halfwidth, int halfheight, int offset);
        void DrawCorrelationOnCircle(QPainter* painterChanel, int halfheight, int offset, std::vector<std::vector<float>> dist, std::vector<std::vector<float>> corre);
        int GetIndexFromElectrodeLabel(std::string myString);
        QColor GetColorFromLabel(std::string label, std::string& memoryLabel);

    private:		
        int m_colorId = -1;
    };
}

#endif