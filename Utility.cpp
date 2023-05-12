#include "Utility.h"

std::vector<std::string> InsermLibrary::readTxtFile(std::string pathFile)
{
	std::stringstream buffer;
    std::ifstream fileStream(pathFile);
    if (fileStream)
	{
        buffer << fileStream.rdbuf();
        fileStream.close();
		return(split<std::string>(buffer.str(), "\r\n"));
	}
	else
	{
		std::cout << " Error opening : " << pathFile << std::endl;
		return std::vector<std::string>();
	}
}

void InsermLibrary::saveTxtFile(std::vector<QString> data, std::string pathFile)
{
    int elementCount = static_cast<int>(data.size());

    std::ofstream fileStream(pathFile, std::ios::out);
    for (int i = 0; i < elementCount; i++)
	{
        fileStream << data[i].toStdString() << std::endl;
	}
    fileStream.close();
}

std::string InsermLibrary::GetCurrentWorkingDir()
{
	char buff[FILENAME_MAX];
	GetCurrentDir(buff, FILENAME_MAX);
	std::string current_working_dir(buff);

	#if defined(_WIN32) || defined(_WIN64)
		replace(current_working_dir.begin(), current_working_dir.end(), '\\', '/');
	#endif
	return current_working_dir;
}
