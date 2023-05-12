#pragma once

#include <iostream>
#include "ITxtFile.h"
#include "FrequencyBand.h"
#include <QCoreApplication>

namespace InsermLibrary
{
	class FrequencyFile : public ITxtFile
	{
	public:
        FrequencyFile(const std::string& filePath = QCoreApplication::applicationDirPath().toStdString() +  "/Resources/Config/frequencyBand.txt");
        inline std::vector<FrequencyBand>& FrequencyBands()
		{
			return m_frequencyBands;
		}
		void Load();
		void Save();

	private:
		std::vector<FrequencyBand> m_frequencyBands;
	};
}
