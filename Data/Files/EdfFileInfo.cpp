#include "EdfFileInfo.h"

InsermLibrary::EdfFileInfo::EdfFileInfo()
{

}

InsermLibrary::EdfFileInfo::EdfFileInfo(std::string edf)
{
    m_edf = edf;
}

InsermLibrary::EdfFileInfo::~EdfFileInfo()
{

}

InsermLibrary::FileType InsermLibrary::EdfFileInfo::GetFileType()
{
    return InsermLibrary::FileType::EuropeanDataFormat;
}

std::vector<std::string> InsermLibrary::EdfFileInfo::GetFiles()
{
    std::vector<std::string> files { FilePath() };
    return files;
}

std::string InsermLibrary::EdfFileInfo::GetFilesString()
{
    return FilePath();
}

int InsermLibrary::EdfFileInfo::CheckForErrors()
{
    std::filesystem::path fspath(FilePath());

    if(fspath.empty())
    {
        return -3;
    }
    else if(fspath.extension() != ".edf")
    {
        return -2;
    }
    else if(!std::filesystem::exists(FilePath()))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
