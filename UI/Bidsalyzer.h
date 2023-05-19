#ifndef BIDSALYZER_H
#define BIDSALYZER_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QMessageBox>
#include "Data/BidsSubject.h"
#include <QReadWriteLock>
#include "Worker/IWorker.h"
#include "Data/Files/FrequencyFile.h"
#include "Data/Files/GeneralOptionsFile.h"
#include "UI/Options/StatisticOptions.h"
#include "UI/Options/PictureOptionsWindow.h"
#include "Worker/BidsSubjectWorker.h"

namespace Ui {
class Bidsalyzer;
}

class Bidsalyzer : public QMainWindow
{
    Q_OBJECT

public:
    explicit Bidsalyzer(QWidget *parent = nullptr);
    ~Bidsalyzer();

private:
    void LoadFrequencyBandsUI(const std::vector<InsermLibrary::FrequencyBand>& FrequencyBands);
    void LoadBidsSubject();
    void LoadTreeViewUI(QString rootFolder);
    bool SeemsToBeBidsSubject(QString rootFolder);
    BidsSubject ParseBidsSubjectInfo(QString rootFolder);
    void ProcessBidsAnalysis();
    int PrepareSubjectFolder();
    void CancelAnalysis();
    void InitProgressBar();
    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> GetUIAnalysisOption();

private slots:
    void DisplayLog(QString info);
    void DisplayColoredLog(QString info, QColor color = QColor(Qt::GlobalColor::black));
    void ReceiveElectrodeList(std::vector<std::string> ElectrodeList, std::string ConnectCleanerFile);

signals:
    void MontageDone(int);

private:
    Ui::Bidsalyzer *ui;
    QFileSystemModel *m_localFileSystemModel = nullptr;
    BidsSubject m_bidsSubject;
    BidsSubject workingCopy;
    InsermLibrary::GeneralOptionsFile *m_GeneralOptionsFile = nullptr;
    InsermLibrary::FrequencyFile m_frequencyFile;
    //==Thread and Worker
    QReadWriteLock m_lockLoop;
    QThread* thread = nullptr;
    IWorker* worker = nullptr;
    bool isAlreadyRunning = false;
    //==Windows
    PictureOptionsWindow *picOpt = nullptr;
    StatisticOptions *optStat = nullptr;
    float nbDoneTask = 0;
    float nbTaskToDo = 0;
};

#endif // BIDSALYZER_H
