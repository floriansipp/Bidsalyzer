#ifndef BLOCWINDOW_H
#define BLOCWINDOW_H

#include "ui_BlocWindow.h"
#include <QtWidgets/QDialog>

//#include <iostream>
//#include <vector>
#include "Bloc.h"
#include "SubBlocWindow.h"

class BlocWindow : public QDialog
{
	Q_OBJECT

public:
    BlocWindow(InsermLibrary::Bloc& bloc, QWidget* parent = nullptr);
	~BlocWindow();

private:
	void LoadSubBlocs();
    void UpdateSubBlocsDisplay(int index);

private slots:
	void OnSubBlocDoubleClicked();
	void AddElement();
	void RemoveElement();
	void OnSubBlocWindowAccepted();
	void OnSubBlocWindowRejected();
	void ValidateModifications();

private:
	Ui::BlocWindow ui;
	InsermLibrary::Bloc* m_bloc = nullptr;
    int m_subBlocIndex = -1;
    InsermLibrary::SubBloc m_memorySubbloc;
};

#endif
