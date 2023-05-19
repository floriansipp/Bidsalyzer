#include "ui_Bidsalyzer.h"
#include "Bidsalyzer.h"
#include <QDebug>
#include <QFileDialog>
#include <QDirIterator>
#include <QStringList>
#include "Data/Files/BrainvisionFileInfo.h"
#include "UI/ConnectCleaner.h"
#include "UI/Options/Protocols/ProtocolsWindow.h"
#include "UI/Options/Frequency/FrequenciesWindow.h"

Bidsalyzer::Bidsalyzer(QWidget *parent) : QMainWindow(parent), ui(new Ui::Bidsalyzer)
{
    ui->setupUi(this);
    //ResetupGUI
    m_GeneralOptionsFile = new InsermLibrary::GeneralOptionsFile();
    m_GeneralOptionsFile->Load();
    m_frequencyFile = InsermLibrary::FrequencyFile();
    m_frequencyFile.Load();
    //== Windows from which we need data (maybe modify to send back the option struct one day)
    optStat = new StatisticOptions();
    picOpt = new PictureOptionsWindow();
    LoadFrequencyBandsUI(m_frequencyFile.FrequencyBands());
    ui->progressBar_3->reset();
    //===end resetup gui

    QAction* openBIDSFolder = ui->menuFile->actions().at(0);
    connect(openBIDSFolder, &QAction::triggered, this, &Bidsalyzer::LoadBidsSubject);
    //===
    QAction* openLocalizerWindow = ui->menuConfiguration->actions().at(0);
    connect(openLocalizerWindow, &QAction::triggered, this, [&]
    {
        ProtocolsWindow* protocolsWindow = new ProtocolsWindow(this);
        protocolsWindow->setAttribute(Qt::WA_DeleteOnClose);
        protocolsWindow->show();
    });

    QAction* openFrequencyBandsWindow = ui->menuConfiguration->actions().at(1);
    connect(openFrequencyBandsWindow, &QAction::triggered, this, [&]
    {
        FrequenciesWindow* frequenciesWindow = new FrequenciesWindow(this);
        connect(frequenciesWindow, &FrequenciesWindow::accepted, this, [&]
        {
            m_frequencyFile = InsermLibrary::FrequencyFile();
            m_frequencyFile.Load();
            LoadFrequencyBandsUI(m_frequencyFile.FrequencyBands());
        });
        frequenciesWindow->setAttribute(Qt::WA_DeleteOnClose);
        frequenciesWindow->show();
    });

    QAction* openPicturesWindow = ui->menuConfiguration->actions().at(2);
    connect(openPicturesWindow, &QAction::triggered, this, [&]
    {
        picOpt->exec();
    });

    QAction* openStatisticsWindow = ui->menuConfiguration->actions().at(3);
    connect(openStatisticsWindow, &QAction::triggered, this, [&]
    {
        optStat->exec();
    });
    //===
    connect(ui->processButton_3, &QPushButton::clicked, this, &Bidsalyzer::ProcessBidsAnalysis);
    connect(ui->cancelButton_3, &QPushButton::clicked, this, &Bidsalyzer::CancelAnalysis);

}

Bidsalyzer::~Bidsalyzer()
{
    delete ui;
}

void Bidsalyzer::LoadFrequencyBandsUI(const std::vector<InsermLibrary::FrequencyBand>& FrequencyBands)
{
    ui->FrequencyListWidget_3->clear();
    for (size_t i = 0; i < FrequencyBands.size(); i++)
    {
        QString Label = QString::fromStdString(FrequencyBands[i].Label());

        QListWidgetItem *currentBand = new QListWidgetItem(ui->FrequencyListWidget_3);
        currentBand->setText(Label);
        currentBand->setFlags(currentBand->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        currentBand->setCheckState(Qt::Unchecked); // AND initialize check state
    }
}

void Bidsalyzer::LoadBidsSubject()
{
    QFileDialog *fileDial = new QFileDialog(this);
    fileDial->setOption(QFileDialog::ShowDirsOnly, true);
    QString fileName = fileDial->getExistingDirectory(this, tr("Choose Bids Subject Folder"));
    if (fileName != "")
    {
        QString rootFolderPath = QDir(fileName).absolutePath();
        LoadTreeViewUI(rootFolderPath);
        bool seemsBids = SeemsToBeBidsSubject(rootFolderPath);
        if(seemsBids)
        {
            //delete m_bidsSubject;
            m_bidsSubject = ParseBidsSubjectInfo(rootFolderPath);

            ui->LocalizerListWidget->clear();
            for(int i = 0; i < m_bidsSubject.Tasks().size(); i++)
            {
                QListWidgetItem *item = new QListWidgetItem(ui->LocalizerListWidget);
                item->setText(QString::fromStdString(m_bidsSubject.Tasks()[i]));
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
                item->setCheckState(Qt::Unchecked); // AND initialize check state
            }
        }
        else
        {
            QMessageBox::information(this, "Error", "It does not seem to be a valid Bids subject, please check your data");
        }
    }
}

void Bidsalyzer::LoadTreeViewUI(QString rootFolder)
{
    //Define file system model at the root folder chosen by the user
    m_localFileSystemModel = new QFileSystemModel();
    m_localFileSystemModel->setReadOnly(true);
    m_localFileSystemModel->setRootPath(rootFolder);

    //set model in treeview
    ui->FileTreeDeselectableView->setModel(m_localFileSystemModel);
    //Show only what is under this path
    ui->FileTreeDeselectableView->setRootIndex(m_localFileSystemModel->index(rootFolder));
    //Show everything put starts at the given model index
    //ui->FileTreeView->setCurrentIndex(m_localFileSystemModel.index(initialFolder));

    //==[Ui Layout]
    ui->FileTreeDeselectableView->setAnimated(false);
    ui->FileTreeDeselectableView->setIndentation(20);
    //Sorting enabled puts elements in reverse (last is first, first is last)
    //ui->FileTreeView->setSortingEnabled(true);
    //Hide name, file size, file type , etc
    ui->FileTreeDeselectableView->hideColumn(1);
    ui->FileTreeDeselectableView->hideColumn(2);
    ui->FileTreeDeselectableView->hideColumn(3);
    ui->FileTreeDeselectableView->header()->hide();
}

bool Bidsalyzer::SeemsToBeBidsSubject(QString rootFolder)
{
    QDir root = QDir(rootFolder);
    bool rootNameOk = root.dirName().contains("sub-");
    bool hasPostSession = false;
    bool hasIeegFolder = false;

    QString NamePostSession = "";
    root.setFilter(QDir::Dirs);
    QStringList entries = root.entryList();
    for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
    {
        QString dirname = *entry;
        if (dirname != QObject::tr(".") && dirname != QObject::tr(".."))
        {
            if(dirname.contains("ses-post"))
            {
                NamePostSession = dirname;
                hasPostSession = true;

                if(hasPostSession)
                {
                    QString postSessionIeegFolderToLookFor = rootFolder + "/" + NamePostSession + "/ieeg";

                    QDirIterator it(rootFolder, QStringList() << "anat" << "ieeg", QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
                    while(it.hasNext())
                    {
                        QString folder = it.next();
                        if(folder == postSessionIeegFolderToLookFor)
                        {
                            hasIeegFolder = true;
                            break;
                        }
                    }
                }
            }
        }
    }

//    qDebug() << rootNameOk;
//    qDebug() << hasPostSession;
//    qDebug() << hasIeegFolder;

    return rootNameOk && hasPostSession && hasIeegFolder;
}

BidsSubject Bidsalyzer::ParseBidsSubjectInfo(QString rootFolder)
{
    std::vector<std::string> tasks;
    std::vector<InsermLibrary::BrainVisionFileInfo> fileInfos;
    QDirIterator it(rootFolder, QStringList() << "*.vhdr", QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        QString vhdr = it.next();
        QStringList split = vhdr.split("_task-");
        QString task = split[1].split("_").first();

        tasks.push_back(task.toStdString());
        fileInfos.push_back(InsermLibrary::BrainVisionFileInfo(vhdr.toStdString()));
    }

    if(tasks.size() == fileInfos.size())
    {
        return BidsSubject(rootFolder.toStdString(), tasks, fileInfos);
    }
    else
    {
        return BidsSubject();
    }
}

void Bidsalyzer::ProcessBidsAnalysis()
{
    if (isAlreadyRunning)
    {
        QMessageBox::critical(this, "Analysis already running", "Please wait until the current analysis if finished");
        return;
    }

    //Get info from ui and check that at least one task can be analyzed
    InsermLibrary::picOption optpic = picOpt->getPicOption();
    InsermLibrary::statOption optstat = optStat->getStatOption();
    InitProgressBar();
    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> analysisOptions = GetUIAnalysisOption();
    //for now no file priority, if we add edf and/or other we will put the system back in place

    int result = PrepareSubjectFolder();
    if(result == -1)
    {
        QMessageBox::critical(this, "No Tasks Selected", "You need to select at least one task to process in the user interface");
        return;
    }

    //If we arrive at this point everything is all good

    thread = new QThread;
    worker = new BidsSubjectWorker(workingCopy, analysisOptions, optstat, optpic, "PtsFilePath");

    //=== Event update displayer
    connect(worker, &IWorker::sendLogInfo, this, &Bidsalyzer::DisplayLog);
    connect(worker->GetLoca(), &InsermLibrary::LOCA::sendLogInfo, this, &Bidsalyzer::DisplayLog);
    //connect(worker->GetLoca(), &InsermLibrary::LOCA::incrementAdavnce, this, &Bidsalyzer::UpdateProgressBar);

    //New ping pong order
    connect(thread, &QThread::started, this, [&]{ worker->ExtractElectrodeList(); });
    connect(worker, &IWorker::sendElectrodeList, this, &Bidsalyzer::ReceiveElectrodeList);
    connect(this, &Bidsalyzer::MontageDone, worker, &IWorker::Process);

    //=== Event From worker and thread
    connect(worker, &IWorker::finished, thread, &QThread::quit);
    connect(worker, &IWorker::finished, worker, &IWorker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(worker, &IWorker::finished, this, [&] { isAlreadyRunning = false; });

    //=== Launch Thread and lock possible second launch
    worker->moveToThread(thread);
    thread->start();
    isAlreadyRunning = true;
}

int Bidsalyzer::PrepareSubjectFolder()
{
    int count = 0;
    std::vector<bool> deleteMe = std::vector<bool>(ui->LocalizerListWidget->count(), true);
    for (int i = 0; i < ui->LocalizerListWidget->count(); i++)
    {
        if (ui->LocalizerListWidget->item(i)->checkState() == Qt::CheckState::Checked)
        {
            deleteMe[i] = false;
            count++;
        }
    }

    if(count == 0) { qDebug() << "no selected "; return -1; }

    workingCopy = BidsSubject(m_bidsSubject);
    int ExamCount = static_cast<int>(deleteMe.size());
    for (int i = ExamCount - 1; i >= 0; i--)
    {
        if (deleteMe[i])
        {
            qDebug() << "Should delete " << ui->LocalizerListWidget->item(i)->text();
            workingCopy.DeleteTask(ui->LocalizerListWidget->item(i)->text().toStdString());
        }
    }

    return 0;
}

void Bidsalyzer::CancelAnalysis()
{
    if (isAlreadyRunning)
    {
        m_lockLoop.lockForWrite();
        thread->quit();
        thread->wait();
        isAlreadyRunning = false;
        QMessageBox::information(this, "Canceled", "Analysis has been canceled by the user");
        DisplayLog("");
        DisplayLog("Analysis has been canceled by the user");
        DisplayLog("");
        m_lockLoop.unlock();
    }
}

void Bidsalyzer::InitProgressBar()
{
    ui->progressBar_3->reset();
    nbDoneTask = 0;
    nbTaskToDo = 0;

    //TODO : corriger ca pour bids ici
    int nbFolderSelected = 1;//GetSelectedFolderCount(ui->FileTreeDeselectableView->selectionModel()->selectedRows());
    int nbFrequencyBands = 0;
    for (int i = 0; i < ui->FrequencyListWidget_3->count(); i++)
    {
        if (ui->FrequencyListWidget_3->item(i)->checkState() == Qt::CheckState::Checked)
            nbFrequencyBands++;
    }

    ui->Eeg2envCheckBox_3->isChecked() ? nbTaskToDo++ : nbTaskToDo++; //eeg2env, wheter we need to compute or load
    ui->Env2plotCheckBox_3->isChecked() ? nbTaskToDo++ : nbTaskToDo;
    ui->TrialmatCheckBox_3->isChecked() ? nbTaskToDo++ : nbTaskToDo;
    ui->CorrelationMapsCheckBox_3->isChecked() ? nbTaskToDo++ : nbTaskToDo;
    ui->StatFileExportCheckBox_3->isChecked() ? nbTaskToDo++ : nbTaskToDo;

    nbTaskToDo *= nbFolderSelected * nbFrequencyBands;
}

std::vector<InsermLibrary::FrequencyBandAnalysisOpt> Bidsalyzer::GetUIAnalysisOption()
{
    std::vector<InsermLibrary::FrequencyBand> frequencyBands = m_frequencyFile.FrequencyBands();
    std::vector<int> indexes;
    for (int i = 0; i < ui->FrequencyListWidget_3->count(); i++)
    {
        if (ui->FrequencyListWidget_3->item(i)->checkState() == Qt::CheckState::Checked)
            indexes.push_back(i);
    }
    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> analysisOpt = std::vector<InsermLibrary::FrequencyBandAnalysisOpt>(indexes.size());

    for (size_t i = 0; i < indexes.size(); i++)
    {
        //	- env2plot and / or trial mat
        analysisOpt[i].analysisParameters.eeg2env2 = ui->Eeg2envCheckBox_3->isChecked();
        analysisOpt[i].analysisParameters.outputType = EEGFormat::FileType::BrainVision; //Blocked for now, will add edf is requiered
        analysisOpt[i].analysisParameters.calculationType = Algorithm::Strategy::GetFileTypeFromString(ui->AnalysisCcomboBox_3->currentText().toStdString());
        analysisOpt[i].env2plot = ui->Env2plotCheckBox_3->isChecked();
        analysisOpt[i].trialmat = ui->TrialmatCheckBox_3->isChecked();
        analysisOpt[i].correMaps = ui->CorrelationMapsCheckBox_3->isChecked();
        analysisOpt[i].statFiles = ui->StatFileExportCheckBox_3->isChecked();

        //TODO : maybe something more clean but since we are not dependant on user input , meh
        std::string smoothingIDStr = ui->StatSmoothingComboBox_3->currentText().toStdString();
        smoothingIDStr.erase(std::remove_if(smoothingIDStr.begin(), smoothingIDStr.end(), [](char c) { return !std::isdigit(c); }), smoothingIDStr.end());
        int num = std::stoi(smoothingIDStr);
        analysisOpt[i].smoothingIDToUse = (num == 0) ? 0 : (num == 250) ? 1 : (num == 500) ? 2 : (num == 1000) ? 3 : (num == 2500) ? 4 : 5;

        //	- what frequency bands data is
        QString label = ui->FrequencyListWidget_3->item(indexes[i])->text();
        std::vector<InsermLibrary::FrequencyBand>::iterator it = std::find_if(frequencyBands.begin(), frequencyBands.end(), [&](const InsermLibrary::FrequencyBand &c)
        {
            return (c.Label() == label.toStdString());
        });
        if (it != frequencyBands.end())
        {
            analysisOpt[i].Band = InsermLibrary::FrequencyBand(*it);
        }
    }

    return analysisOpt;
}

//===Slots
void Bidsalyzer::DisplayLog(QString messageToDisplay)
{
    ui->messageDisplayer->append(messageToDisplay);
}

void Bidsalyzer::DisplayColoredLog(QString messageToDisplay, QColor color)
{
    ui->messageDisplayer->setTextColor(color);
    DisplayLog(messageToDisplay);
    ui->messageDisplayer->setTextColor(Qt::GlobalColor::black);
}

void Bidsalyzer::ReceiveElectrodeList(std::vector<std::string> ElectrodeList, std::string ConnectCleanerFile)
{
    if(QFileInfo::exists(ConnectCleanerFile.c_str()))
    {
//        std::vector<std::string> uncorrectedLabels;
//        std::vector<int> states;
//        std::vector<std::string> correctedLabels;
//        LoadCCFFile(ConnectCleanerFile.c_str(), uncorrectedLabels, states, correctedLabels);
//        worker->SetExternalParameters(states, correctedLabels, 1);
//        emit MontageDone(1);
    }
    else
    {
        ConnectCleaner *elecWin = new ConnectCleaner(ElectrodeList, ConnectCleanerFile.c_str(), nullptr);
        int res = elecWin->exec();
        if(res == 1)
        {
            worker->SetExternalParameters(elecWin->IndexToDelete(), elecWin->CorrectedLabel(), elecWin->OperationToDo());
            emit MontageDone(res);
        }
        else
        {
            CancelAnalysis();
        }
        delete elecWin;
    }
}
