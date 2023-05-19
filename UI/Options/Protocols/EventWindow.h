#ifndef EVENTWINDOW_H
#define EVENTWINDOW_H

#include "ui_EventWindow.h"
#include <QtWidgets/QDialog>

#include "Event.h"

class EventWindow : public QDialog
{
	Q_OBJECT

public:
    EventWindow(InsermLibrary::Event& sbevent, QWidget* parent = nullptr);
	~EventWindow();

private:
	void SetupComboBoxType();

private slots:
	void ValidateModifications();

private:
    InsermLibrary::Event* m_Event = nullptr;
	Ui::EventWindow ui;
};

#endif
