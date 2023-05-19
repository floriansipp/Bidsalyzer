#ifndef PROTOCOLSWINDOW_H
#define PROTOCOLSWINDOW_H

#include "ui_ProtocolsWindow.h"
#include <QtWidgets/QDialog>
#include <QDir>
#include <QList>
#include <QString>
#include <iostream>

#include "ProvFile.h"

class ProtocolsWindow : public QDialog
{
	Q_OBJECT

public:
	ProtocolsWindow(QWidget *parent = nullptr);
    ~ProtocolsWindow();

private:
	QStringList GetProtocolsFileList();
    void LoadProtocols(std::vector<InsermLibrary::ProvFile> protocols);

private slots:
	void OnProtocolDoubleClicked(QListWidgetItem* item);
	void AddElement();
	void RemoveElement();
	void OnProtocolWindowAccepted();
	void OnProtocolWindowRejected();
    void ValidateAndSave();

private:
	Ui::ProtocolsWindow ui;
	QString m_ProtocolFolder = QCoreApplication::applicationDirPath() + "/Resources/Config/Prov";
    std::vector<InsermLibrary::ProvFile> m_ProvFiles;
    std::vector<bool> m_ProvFileModifed;
    InsermLibrary::ProvFile m_memoryFile;
    int m_fileIndex = -1;

};

#endif
