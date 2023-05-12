#include "GeneralOptionsFile.h"

InsermLibrary::GeneralOptionsFile::GeneralOptionsFile(const std::string& filePath) : ITxtFile(filePath)
{

}

void InsermLibrary::GeneralOptionsFile::Load()
{
	int LineCount = m_rawTextFileData.size();
	for (int i = 0; i < LineCount; i++)
	{
		if (i == 0) 
		{
            m_fileExtensions = std::vector<InsermLibrary::FileType>();
			std::vector <std::string> RawFileOrderList = EEGFormat::Utility::Split<std::string>(m_rawTextFileData[i], "-");
			for (int j = 0; j < RawFileOrderList.size(); j++)
			{
				if (RawFileOrderList[j].compare("Micromed") == 0)
				{
                    m_fileExtensions.push_back(InsermLibrary::FileType::Micromed);
				}
				else if (RawFileOrderList[j].compare("Elan") == 0)
				{
                    m_fileExtensions.push_back(InsermLibrary::FileType::Elan);
				}
				else if (RawFileOrderList[j].compare("BrainVision") == 0)
				{
                    m_fileExtensions.push_back(InsermLibrary::FileType::Brainvision);
				}
				else if (RawFileOrderList[j].compare("Edf") == 0)
				{
                    m_fileExtensions.push_back(InsermLibrary::FileType::EuropeanDataFormat);
				}
				else
				{
					std::cout << "GeneralOptionsFile::Load() => " << RawFileOrderList[j] << " not supported" << std::endl;
				}
			}
		}
	}
}

void InsermLibrary::GeneralOptionsFile::Save()
{
	std::ofstream optionFileStream(m_originalFilePath, std::ios::out);
	for (int i = 0; i < m_fileExtensions.size(); i++)
	{
		switch (m_fileExtensions[i])
		{
            case InsermLibrary::FileType::Micromed:
			{
				optionFileStream << "Micromed";
				break;
			}
            case InsermLibrary::FileType::Elan:
			{
				optionFileStream << "Elan";
				break;
			}
            case InsermLibrary::FileType::Brainvision:
			{
				optionFileStream << "BrainVision";
				break;
			}
            case InsermLibrary::FileType::EuropeanDataFormat:
			{
				optionFileStream << "Edf";
				break;
			}
		}

		if (i < m_fileExtensions.size() - 1)
		{
			optionFileStream << "-";
		}
	}
	optionFileStream.close();
}
