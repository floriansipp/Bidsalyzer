#ifndef FREQUENCIESWINDOW_H
#define FREQUENCIESWINDOW_H

#include "ui_FrequenciesWindow.h"
#include <QtWidgets/QDialog>
#include "Data/Files/FrequencyFile.h"
#include <QListWidgetItem>
#include <iostream>
#include <vector>

class FrequenciesWindow : public QDialog
{
    Q_OBJECT

public:
    FrequenciesWindow(QWidget *parent = nullptr);
    ~FrequenciesWindow();

private :
    void LoadFrequencyBands(const std::vector<InsermLibrary::FrequencyBand>& FrequencyBands);

private slots:
    void OnFrequencyDoubleClicked(QListWidgetItem* item);
    void AddElement();
    void RemoveElement();
    void OnFrequencyWindowAccepted();
    void OnFrequencyWindowRejected();
    void ValidateModifications();

private:
    Ui::FrequenciesWindow ui;
    InsermLibrary::FrequencyFile* f = nullptr;
    int m_FrequencyIndex = -1;
    InsermLibrary::FrequencyBand m_MemoryFrequency;
};

#endif // FREQUENCIESWINDOW_H
