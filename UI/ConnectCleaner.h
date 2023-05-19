#ifndef CONNECT_CLEANER_H
#define CONNECT_CLEANER_H

#include "ui_ConnectCleaner.h"
#include <QtWidgets/QDialog>
#include <QLineEdit>
#include <iostream>
#include <string>
#include <vector>
#include "../Data/Files/CleanConnectFile.h"
#include <QTableView>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QAbstractItemModel>
#include <QKeyEvent>

class ConnectCleaner : public QDialog
{
	Q_OBJECT

public:
    ConnectCleaner(std::vector<std::string> ElectrodeList, QString connectCleanerFilePath, QWidget *parent = nullptr);
	~ConnectCleaner();

    inline std::vector<int> IndexToDelete() { return m_indexToDelete; }
    inline std::vector<std::string> UncorrectedLabels() { return m_ElectrodesLabel; }
    inline std::vector<std::string> CorrectedLabel() { return m_CorrectedElectrodesLabel; }
    inline int OperationToDo() { if(ui.BipoleCheckBox->isChecked()) return 1; else return 0; }

private:
    void FillList(const std::vector<std::string> & labels);
    void keyPressEvent(QKeyEvent *);

private slots:
    /**
     * @brief When Multiple elements are selected, change all of their state according
     * to the one being modified either by mouse click or key press
     */
    void CheckMultipleItems(QStandardItem *item);
    /**
     * @brief Get the id of unwanted electrodes and correct the labeling of wanted one
     */
    void ValidateConnect();

private:
	std::vector<std::string> m_ElectrodesLabel;
    std::vector<std::string> m_CorrectedElectrodesLabel;
    std::vector<int> m_indexToDelete;
	Ui::ConnectCleanerForm ui;
    bool m_lockMultiple = false;
    QString m_connectCleanerFilePath = "";
    CleanConnectFile *m_cleanConnectFile = nullptr;
};

#endif
