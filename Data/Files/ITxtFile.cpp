#include "ITxtFile.h"

InsermLibrary::ITxtFile::ITxtFile(const std::string& filePath)
{
	m_originalFilePath = filePath;
	m_rawTextFileData = ReadTxtFile(filePath);
}

InsermLibrary::ITxtFile::~ITxtFile()
{

}

std::vector<std::string> InsermLibrary::ITxtFile::ReadTxtFile(const std::string& pathFile)
{
	std::stringstream buffer;
	if (EEGFormat::Utility::IsValidFile(pathFile))
	{
		std::ifstream file(pathFile);
		buffer << file.rdbuf();
		file.close();
		return(EEGFormat::Utility::Split<std::string>(buffer.str(), "\r\n"));
	}
	else
	{
		return std::vector<std::string>();
	}
}
