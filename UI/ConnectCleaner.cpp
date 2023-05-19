#include "ConnectCleaner.h"

ConnectCleaner::ConnectCleaner(std::vector<std::string> ElectrodeList, QString connectCleanerFilePath, QWidget *parent) : QDialog(parent)
{
    m_ElectrodesLabel = std::vector<std::string>(ElectrodeList);
    m_connectCleanerFilePath = connectCleanerFilePath;

    ui.setupUi(this);

    connect(ui.MonoCheckBox, &QCheckBox::clicked, this, [&]
    {
        if(ui.MonoCheckBox->isChecked())
        {
            ui.BipoleCheckBox->setChecked(false);
        }
        else
        {
            ui.BipoleCheckBox->setChecked(true);
        }
    });
    connect(ui.BipoleCheckBox, &QCheckBox::clicked, this, [&]
    {
        if(ui.BipoleCheckBox->isChecked())
        {
            ui.MonoCheckBox->setChecked(false);
        }
        else
        {
            ui.MonoCheckBox->setChecked(true);
        }
    });
    connect(ui.ValidateButton, &QPushButton::clicked, this, &ConnectCleaner::ValidateConnect);
    connect(ui.ExportButton, &QPushButton::clicked, this, [&]{ m_cleanConnectFile->Save(); });
    FillList(m_ElectrodesLabel);
}

ConnectCleaner::~ConnectCleaner()
{
    if(m_cleanConnectFile != nullptr)
    {
        delete m_cleanConnectFile;
        m_cleanConnectFile = nullptr;
    }
}

void ConnectCleaner::FillList(const std::vector<std::string> & labels)
{
    //Create Electrode list from file and/or labels
    m_cleanConnectFile = new CleanConnectFile(m_connectCleanerFilePath, labels);

    //Connect model to detect click
    connect(m_cleanConnectFile->Model(), &QStandardItemModel::itemChanged, this, &ConnectCleaner::CheckMultipleItems);

    //Put Model in view
    ui.SelectElectrodeList->setModel(m_cleanConnectFile->Model());
    ui.SelectElectrodeList->resizeColumnsToContents();
    ui.SelectElectrodeList->resizeRowsToContents();
    ui.SelectElectrodeList->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    ui.SelectElectrodeList->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    ui.SelectElectrodeList->horizontalHeader()->setHidden(true);
    ui.SelectElectrodeList->verticalHeader()->setHidden(true);
}

void ConnectCleaner::keyPressEvent(QKeyEvent *key)
{
    if ( (key->key()==Qt::Key_Enter) || (key->key()==Qt::Key_Return) )
    {
        QModelIndex index = ui.SelectElectrodeList->selectionModel()->currentIndex();
        QStandardItem *item = dynamic_cast<QStandardItemModel*>(ui.SelectElectrodeList->model())->itemFromIndex(index);
        if(item != nullptr)
        {
            if (item->checkState() == Qt::CheckState::Checked)
            {
                item->setCheckState(Qt::CheckState::Unchecked);
            }
            else
            {
                item->setCheckState(Qt::CheckState::Checked);
            }
        }
    }
}

void ConnectCleaner::CheckMultipleItems(QStandardItem *item)
{
    if(m_lockMultiple == true)
        return;

    m_lockMultiple = true;

    QModelIndexList selection = ui.SelectElectrodeList->selectionModel()->selectedIndexes();
    QStandardItemModel *qsim = dynamic_cast<QStandardItemModel*>(ui.SelectElectrodeList->model());
    for (int i = 0; i < selection.size(); i++)
    {
        qsim->item(selection[i].row(), 0)->setCheckState(item->checkState());
    }

    m_lockMultiple = false;
}

void ConnectCleaner::ValidateConnect()
{
    m_indexToDelete = std::vector<int>();
    m_CorrectedElectrodesLabel = std::vector<std::string>();

    QStandardItemModel *qsim = dynamic_cast<QStandardItemModel*>(ui.SelectElectrodeList->model());
    for (int i = 0; i < qsim->rowCount(); i++)
    {
        if (qsim->item(i, 0)->checkState() == Qt::CheckState::Unchecked)
        {
            m_indexToDelete.push_back(i);
        }
        std::string label = qsim->item(i, 1)->text().toStdString();
        m_CorrectedElectrodesLabel.push_back(label);
    }
    done(1);
    close();
}
