#ifndef CLEANCONNECTFILE_H
#define CLEANCONNECTFILE_H

#include <iostream>     // std::cout, std::endl
#include <iomanip>      // std::setw
#include <string>
#include <vector>
#include "../../EEGFormat/EEGFormat/Utility.h"
#include <QFileInfo>
#include <QTextStream>
#include <QStandardItemModel>
#include <QRegularExpression>
#include <QString>

class CleanConnectFile
{
public:
    CleanConnectFile(const QString & filePath, const std::vector<std::string> & labels);
    ~CleanConnectFile();
    inline QStandardItemModel* Model() const { return m_ItemModel; }
    inline QFileInfo FileInfo() const { return m_FileInfo; }
    void Save();

private:
    void Load(QString filePath, std::vector<std::string> & labels,
                                std::vector<Qt::CheckState> & states,
                                std::vector<std::string> & correctedLabels);
    void LoadDataInModel(const std::vector<std::string> & labels,
                         std::vector<Qt::CheckState> states = std::vector<Qt::CheckState>(),
                         std::vector<std::string> correctedLabels = std::vector<std::string>());
    QString CorrectElectrodeNaming(std::string label, bool shouldCorrect);
private:
    QFileInfo m_FileInfo;
    QStandardItemModel* m_ItemModel;
};

#endif // CLEANCONNECTFILE_H
