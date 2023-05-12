#include "CleanConnectFile.h"

CleanConnectFile::CleanConnectFile(const QString & filePath, const std::vector<std::string> & labels)
{
    m_FileInfo = QFileInfo(filePath);
    if(m_FileInfo.exists())
    {
        std::vector<std::string> uncorrectedLabels;
        std::vector<Qt::CheckState> states;
        std::vector<std::string> correctedLabels;
        Load(m_FileInfo.absoluteFilePath(), uncorrectedLabels, states, correctedLabels);
        LoadDataInModel(uncorrectedLabels, states, correctedLabels);
    }
    else
    {
        LoadDataInModel(labels);
    }
}

CleanConnectFile::~CleanConnectFile()
{

}

void CleanConnectFile::Save()
{
    std::ofstream eventsFileStream(m_FileInfo.absoluteFilePath().toStdString());
    for(int i = 0; i < m_ItemModel->rowCount(); i++)
    {
        Qt::CheckState checkState = m_ItemModel->item(i, 0)->checkState();
        QString checkStateLabel = QVariant(checkState).toString();
        QString uncorrectedLabel = m_ItemModel->item(i, 0)->text();
        QString correctLabel = m_ItemModel->item(i, 1)->text();

        if(checkState == Qt::CheckState::Checked)
        {
            eventsFileStream << checkStateLabel.toStdString() << std::setw(20) << uncorrectedLabel.toStdString() << std::setw(20) << correctLabel.toStdString() << std::endl;
        }
        else
        {
            eventsFileStream << checkStateLabel.toStdString()  << std::setw(20) << uncorrectedLabel.toStdString() << std::setw(20) << uncorrectedLabel.toStdString() << std::endl;
        }
    }
    eventsFileStream.close();
}

void CleanConnectFile::Load(QString filePath, std::vector<std::string> & uncorrectedLabels, std::vector<Qt::CheckState> & states, std::vector<std::string> & correctedLabels)
{
    std::vector<std::string> rawFile = EEGFormat::Utility::ReadTextFile(filePath.toStdString());
    for (uint i = 0; i < rawFile.size(); i++)
    {
        std::vector<std::string> rawLine = EEGFormat::Utility::Split<std::string>(rawFile[i], " ");
        if(rawLine.size() != 3)
        {
            std::cerr << "Error, line from connect cleaner file should contain three elements" << std::endl;
            continue;
        }

        Qt::CheckState state = QVariant(rawLine[0].c_str()).toBool() == true ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
        states.push_back(state);
        uncorrectedLabels.push_back(rawLine[1]);
        correctedLabels.push_back(rawLine[2]);
    }
}

void CleanConnectFile::LoadDataInModel(const std::vector<std::string> & labels, std::vector<Qt::CheckState> states, std::vector<std::string> correctedLabels)
{
    bool ShouldCorrectNaming = states.size() == 0 || correctedLabels.size() == 0;
    //Check if this is data from a connect cleaner file or the default states
    if(states.size() == 0)
        states = std::vector<Qt::CheckState>(labels.size(), Qt::CheckState::Checked);
    if(correctedLabels.size() == 0)
        correctedLabels = std::vector<std::string>(labels);

    size_t elementCount = labels.size();
    m_ItemModel = new QStandardItemModel(static_cast<int>(elementCount), 2);
    for (uint i = 0; i < elementCount; i++)
    {
        QStandardItem *check_item = new QStandardItem(QString::fromStdString(labels[i]));
        check_item->setCheckable(true);
        check_item->setCheckState(states[i]);
        check_item->setFlags(check_item->flags() | Qt::ItemIsUserCheckable); //Allow to intercept enter key for modification
        check_item->setFlags(check_item->flags() ^ Qt::ItemIsEditable); // Item not editable since we have correctedLabel_item

        QStandardItem *correctedLabel_item = new QStandardItem(CorrectElectrodeNaming(correctedLabels[i], ShouldCorrectNaming));
        correctedLabel_item->setCheckable(false);

        m_ItemModel->setItem(static_cast<int>(i), 0, check_item);
        m_ItemModel->setItem(static_cast<int>(i), 1, correctedLabel_item);
    }
}

QString CleanConnectFile::CorrectElectrodeNaming(std::string label, bool shouldCorrect)
{
    std::string fixedLabel = std::string(label);
    if(shouldCorrect)
    {
        EEGFormat::Utility::FixElectrodeName(fixedLabel);
        QString correctedLabel = QString::fromStdString(fixedLabel);
        if (correctedLabel.contains(QRegularExpression("^[A-Z]+\'*[0-9]{1,2}$")))
        {
            return correctedLabel;
        }
        return QString::fromStdString("BAD LABELING");
    }
    return QString::fromStdString(fixedLabel);
}
