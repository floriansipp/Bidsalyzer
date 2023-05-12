#ifndef EDFFILEINFO_H
#define EDFFILEINFO_H

#include "IEegFileInfo.h"
#include <iostream>
#include <vector>
#include <filesystem>

namespace InsermLibrary
{
    class EdfFileInfo : public InsermLibrary::IEegFileInfo
    {
    public:
        EdfFileInfo();
        EdfFileInfo(std::string edf);
        ~EdfFileInfo();

        inline std::string const FilePath() { return m_edf; }

        FileType GetFileType();
        std::vector<std::string> GetFiles();
        std::string GetFilesString();
        int CheckForErrors();

    private:
        std::string m_edf = "";
    };
}

#endif // EDFFILEINFO_H
