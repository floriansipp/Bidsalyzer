#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>	
#include "../../EEGFormat/EEGFormat/Utility.h"

namespace InsermLibrary
{
	class ITxtFile
	{
	public:
		ITxtFile(const std::string& filePath);
        virtual ~ITxtFile() = 0;
		virtual void Save() = 0;
		virtual void Load() = 0;
	private:
		std::vector<std::string> ReadTxtFile(const std::string& path);
	protected:
		std::string m_originalFilePath;
		std::vector<std::string> m_rawTextFileData;
	};
}
