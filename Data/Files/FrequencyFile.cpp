#include "FrequencyFile.h"

InsermLibrary::FrequencyFile::FrequencyFile(const std::string& filePath) : ITxtFile(filePath)
{

}

void InsermLibrary::FrequencyFile::Load()
{
	int LineCount = m_rawTextFileData.size();
	for (int i = 0; i < LineCount; i+=2)
	{
		std::string bandName = m_rawTextFileData[i];

		if (i + 1 < LineCount)
		{
			std::vector<std::string> splitValue = EEGFormat::Utility::Split<std::string>(m_rawTextFileData[i + 1], ":");
			if (splitValue.size() == 3)
			{
				int fMin = atoi(splitValue[0].c_str());
				int step = atoi(splitValue[1].c_str());
				int fMax = atoi(splitValue[2].c_str());

				m_frequencyBands.push_back(FrequencyBand(bandName, fMin, fMax, step));
			}
		}
	}
}

void InsermLibrary::FrequencyFile::Save()
{
    std::ofstream frequencyFile(m_originalFilePath, std::ios::out);
    if (frequencyFile.is_open())
    {
        for(int i = 0; i < m_frequencyBands.size(); i++)
        {
            frequencyFile << m_frequencyBands[i].Label() << std::endl;
            frequencyFile << m_frequencyBands[i].FMin() << ":" << m_frequencyBands[i].Step() << ":" << m_frequencyBands[i].FMax() << std::endl;
        }
        frequencyFile.close();
    }
    else
    {
        throw std::runtime_error("FrequencyFile Save => : Could not open file");
    }
}
