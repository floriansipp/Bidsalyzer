#ifndef _TRIGGER_H
#define _TRIGGER_H

#include "ITrigger.h"

namespace InsermLibrary
{
	class Trigger
	{
	public:
		Trigger(EEGFormat::ITrigger mainEvent, int samplingFrequency);
		Trigger(EEGFormat::ITrigger mainEvent, EEGFormat::ITrigger response, int samplingFrequency);
		~Trigger();
		inline const EEGFormat::ITrigger MainEvent() const
		{
			return m_mainEvent;
		}
		inline void MainEvent(int code, long sample)
		{
			m_mainEvent.Code(code);
			m_mainEvent.Sample(sample);
		}
		inline void MainEvent(EEGFormat::ITrigger mainEvent)
		{
			m_mainEvent.Code(mainEvent.Code());
			m_mainEvent.Sample(mainEvent.Sample());
		}
		inline const EEGFormat::ITrigger Response () const
		{
			return m_response;
		}
		inline void Response(int code, long sample)
		{
			m_response.Code(code);
			m_response.Sample(sample);
		}
		inline void Response(EEGFormat::ITrigger response)
		{
			m_response.Code(response.Code());
			m_response.Sample(response.Sample());
		}
		inline int SamplingFrequency()
		{
			return m_samplingFrequency;
		}
		inline int MainCode()
		{
			return m_mainEvent.Code();
		}
		inline void MainCode(const int& code)
		{
			return m_mainEvent.Code(code);
		}
		inline long MainSample()
		{
			return m_mainEvent.Sample();
		}
		inline void MainSample(const long& sample)
		{
			return m_mainEvent.Sample(sample);
		}
		inline int ResponseCode()
		{
			return m_response.Code();
		}
		inline void ResponseCode(const int& code)
		{
			return m_response.Code(code);
		}
		inline long ResponseSample()
		{
            return m_response.Sample();
		}
		inline void ResponseSample(const long& sample)
		{
			return m_response.Sample(sample);
		}
		inline int ReactionTimeInSample()
		{
			return m_response.Sample() - m_mainEvent.Sample();
		}
		inline int ReactionTimeInMilliSeconds()
		{
			int mainTime = 1000 * ((float)m_mainEvent.Sample() / m_samplingFrequency);
			int responseTime = 1000 * ((float)m_response.Sample() / m_samplingFrequency);
			return responseTime - mainTime;
		}
		void UpdateFrequency(int newFrequency);
		void SwapStimulationAndResponse();

	private:
		int m_samplingFrequency = 0;
		EEGFormat::ITrigger m_mainEvent;
		EEGFormat::ITrigger m_response;
	};

	inline bool operator==(int code, Trigger &a)
	{
		return a.MainCode() == code;
	}
	inline bool operator!=(int code, Trigger &a)
	{
		return !(a.MainCode() == code);
	}
}
#endif
