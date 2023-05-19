#ifndef ICONWINDOW_H
#define ICONWINDOW_H

#include "ui_IconWindow.h"
#include <QtWidgets/QDialog>
#include <QPixmap>
#include <QFileDialog>
#include "Icon.h"

class IconWindow : public QDialog
{
	Q_OBJECT

public:
    IconWindow(InsermLibrary::Icon& sbicon, QWidget* parent = nullptr);
    ~IconWindow();

//private:

private slots:
    void BrowseIcon();
	void ValidateModifications();

private:
    Ui::IconWindow ui;
    InsermLibrary::Icon* m_Icon = nullptr;
    std::string m_imagePath = "";
};

#endif
