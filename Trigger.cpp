#include "Trigger.h"

InsermLibrary::Trigger::Trigger(EEGFormat::ITrigger mainEvent, int samplingFrequency)
{
	m_mainEvent.Code(mainEvent.Code());
	m_mainEvent.Sample(mainEvent.Sample());
	m_samplingFrequency = samplingFrequency;
	m_response.Code(-1);
}

InsermLibrary::Trigger::Trigger(EEGFormat::ITrigger mainEvent, EEGFormat::ITrigger response, int samplingFrequency)
{
	m_mainEvent.Code(mainEvent.Code());
	m_mainEvent.Sample(mainEvent.Sample());
	m_response.Code(response.Code());
	m_response.Sample(response.Sample());
	m_samplingFrequency = samplingFrequency;
}

InsermLibrary::Trigger::~Trigger()
{

}

void InsermLibrary::Trigger::UpdateFrequency(int newFrequency)
{
	if (newFrequency <= 0)
        throw new std::runtime_error("Error : Sampling Frequency of a trigger must be bigger than zero");

	float Factor = (float)newFrequency / m_samplingFrequency;
	
	int oldSampleValue = m_mainEvent.Sample();
	m_mainEvent.Sample(oldSampleValue * Factor);

	oldSampleValue = m_response.Sample();
	m_response.Sample(oldSampleValue * Factor);

	m_samplingFrequency = newFrequency;
}

void InsermLibrary::Trigger::SwapStimulationAndResponse()
{
	std::swap(m_mainEvent, m_response);
}
