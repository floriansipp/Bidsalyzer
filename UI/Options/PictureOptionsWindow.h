#ifndef PICOPTIONS_H
#define PICOPTIONS_H

#include <QtWidgets/QDialog>
#include "ui_PictureOptionsWindow.h"
#include "Utility.h"
#include "optionsParameters.h"
#include <cmath>

class PictureOptionsWindow : public QDialog
{
	Q_OBJECT

public:
    PictureOptionsWindow(QWidget *parent = 0);
    ~PictureOptionsWindow();
	void connectSignals();
    InsermLibrary::picOption getPicOption();

private slots:
	void width2HeigthTrialRatio();
	void heigth2WidthTrialRatio();
	void width2HeigthPlotRatio();
	void heigth2WidthPlotRatio();

private:
	QSize memTrialmap, memPlot;
	Ui::FormOptPic ui;
};

#endif
