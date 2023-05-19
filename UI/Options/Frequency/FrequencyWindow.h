#ifndef FREQUENCYWINDOW_H
#define FREQUENCYWINDOW_H

#include "ui_FrequencyWindow.h"
#include <QtWidgets/QDialog>
#include <QLineEdit>
#include "Data/FrequencyBand.h"

class FrequencyWindow : public QDialog
{
    Q_OBJECT

public:
    FrequencyWindow(InsermLibrary::FrequencyBand& f, QWidget *parent = nullptr);
    ~FrequencyWindow();

//private :
//    void LoadFrequencyBands(const std::vector<InsermLibrary::FrequencyBand>& FrequencyBands);

private slots:
    void ValidateAndSave();
//    void AddElement();
//    void RemoveElement();
//    void OnFrequencyWindowAccepted();
//    void OnFrequencyWindowRejected();
//    void ValidateModifications();

private:
    Ui::FrequencyWindow ui;
    InsermLibrary::FrequencyBand *m_frequencyBand = nullptr;
};

#endif // FREQUENCIESWINDOW_H
