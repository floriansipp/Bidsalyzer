#pragma once

#include <iostream>
#include "Utility.h"
#include "ITxtFile.h"
#include "IEegFileInfo.h"
#include <QCoreApplication>

//TODO : update so that generalOptionsFile is a qt model interface and create a model class to handle this
namespace InsermLibrary
{
	class GeneralOptionsFile : public ITxtFile
	{
	public:
		GeneralOptionsFile(const std::string& filePath = QCoreApplication::applicationDirPath().toStdString() + "/Resources/Config/generalOptions.txt");	
        inline std::vector<InsermLibrary::FileType>& FileExtensionsFavorite()
		{
			return m_fileExtensions;
		}
		void Load();
		void Save();

	private:
        std::vector<InsermLibrary::FileType> m_fileExtensions;
	};
}
