#ifndef BRAINVISIONFILEINFO_H
#define BRAINVISIONFILEINFO_H

#include "IEegFileInfo.h"
#include <iostream>
#include <vector>
#include <filesystem>

namespace InsermLibrary
{
    class BrainVisionFileInfo : public InsermLibrary::IEegFileInfo
    {
    public:
        BrainVisionFileInfo();
        BrainVisionFileInfo(std::string bvheader);
        ~BrainVisionFileInfo();

        inline std::string const Header() { return m_BvHeader; }

        FileType GetFileType();
        std::vector<std::string> GetFiles();
        std::string GetFilesString();
        int CheckForErrors();

    private:
        std::string m_BvHeader = "";
    };
}

#endif // BRAINVISIONFILEINFO_H
