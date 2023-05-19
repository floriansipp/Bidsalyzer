#include "BrainvisionFileInfo.h"

InsermLibrary::BrainVisionFileInfo::BrainVisionFileInfo()
{

}

InsermLibrary::BrainVisionFileInfo::BrainVisionFileInfo(std::string bvheader)
{
    m_BvHeader = bvheader;
}

InsermLibrary::BrainVisionFileInfo::~BrainVisionFileInfo()
{

}

InsermLibrary::FileType InsermLibrary::BrainVisionFileInfo::GetFileType()
{
    return InsermLibrary::FileType::Brainvision;
}

std::vector<std::string> InsermLibrary::BrainVisionFileInfo::GetFiles()
{
    std::vector<std::string> files { Header() };
    return files;
}

std::string InsermLibrary::BrainVisionFileInfo::GetFilesString()
{
    return Header();
}


int InsermLibrary::BrainVisionFileInfo::CheckForErrors()
{
    std::filesystem::path fspath(Header());

    if(fspath.empty())
    {
        return -3;
    }
    else if(fspath.extension() != ".vhdr")
    {
        return -2;
    }
    else if(!std::filesystem::exists(Header()))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
