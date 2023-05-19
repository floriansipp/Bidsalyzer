#include "Data/BidsSubject.h"
#include <QDebug>

BidsSubject::BidsSubject()
{

}

BidsSubject::BidsSubject(std::string root, std::vector<std::string> tasks, std::vector<InsermLibrary::BrainVisionFileInfo> fileInfos)
{
    m_rootFolder = root;
    m_tasks = std::vector<std::string>(tasks);
    m_fileInfos = std::vector<InsermLibrary::BrainVisionFileInfo>(fileInfos);
}

BidsSubject::~BidsSubject()
{

}

void BidsSubject::DeleteTask(std::string label)
{
    auto it = std::find_if(m_tasks.begin(), m_tasks.end(), [&](const std::string& t) { return t == label; });
    int indexPos = std::distance(m_tasks.begin(), it);
    if(indexPos >= 0)
    {
        m_tasks.erase(m_tasks.begin() + indexPos);
        m_fileInfos.erase(m_fileInfos.begin() + indexPos);
    }
    else
    {
        qDebug() << "Error trying to remove " << label.c_str();
    }
}
