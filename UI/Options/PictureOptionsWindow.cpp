#include "PictureOptionsWindow.h"

PictureOptionsWindow::PictureOptionsWindow(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	connectSignals();
	memTrialmap.setWidth(atoi(ui.widthTrialLE->text().toStdString().c_str()));
	memTrialmap.setHeight(atoi(ui.heigthTrialLE->text().toStdString().c_str()));
	memPlot.setWidth(atoi(ui.widthPlotLE->text().toStdString().c_str()));
	memPlot.setHeight(atoi(ui.heigthPlotLE->text().toStdString().c_str()));
}

PictureOptionsWindow::~PictureOptionsWindow()
{

}

void PictureOptionsWindow::connectSignals()
{
    connect(ui.widthTrialLE, &QLineEdit::editingFinished, this, &PictureOptionsWindow::width2HeigthTrialRatio);
    connect(ui.heigthTrialLE, &QLineEdit::editingFinished, this, &PictureOptionsWindow::heigth2WidthTrialRatio);
    connect(ui.widthPlotLE, &QLineEdit::editingFinished, this, &PictureOptionsWindow::width2HeigthPlotRatio);
    connect(ui.heigthPlotLE, &QLineEdit::editingFinished, this, &PictureOptionsWindow::heigth2WidthPlotRatio);
	connect(ui.pushButton, &QPushButton::clicked, this, [&] { close(); });
}

InsermLibrary::picOption PictureOptionsWindow::getPicOption()
{
    InsermLibrary::picOption picOpt;
	picOpt.sizeTrialmap.setWidth(atoi(ui.widthTrialLE->text().toStdString().c_str()));
	picOpt.sizeTrialmap.setHeight(atoi(ui.heigthTrialLE->text().toStdString().c_str()));
	picOpt.sizePlotmap.setWidth(atoi(ui.widthPlotLE->text().toStdString().c_str()));
	picOpt.sizePlotmap.setHeight(atoi(ui.heigthPlotLE->text().toStdString().c_str()));
	picOpt.interpolationtrialmap.setWidth(atoi(ui.horizInterpoLE->text().toStdString().c_str()));
	picOpt.interpolationtrialmap.setHeight(atoi(ui.vertiInterpoLE->text().toStdString().c_str()));
	return picOpt;
}

void PictureOptionsWindow::width2HeigthTrialRatio()
{
	int width = atoi(ui.widthTrialLE->text().toStdString().c_str());
	if (memTrialmap.width() != width)
	{
		memTrialmap.setWidth(width);
		ui.heigthTrialLE->setText(QString::number(ceil(0.75 * width)));
	}
}

void PictureOptionsWindow::heigth2WidthTrialRatio()
{
	int heigth = atoi(ui.heigthTrialLE->text().toStdString().c_str());
	if (memTrialmap.height() != heigth)
	{
		memTrialmap.setHeight(heigth);
		ui.widthTrialLE->setText(QString::number(ceil(1.333 * heigth)));
	}
}

void PictureOptionsWindow::width2HeigthPlotRatio()
{
	int width = atoi(ui.widthPlotLE->text().toStdString().c_str());
	if (memPlot.width() != width)
	{
		memPlot.setWidth(width);
		ui.heigthPlotLE->setText(QString::number(ceil(0.5921875 * width)));
	}
}

void PictureOptionsWindow::heigth2WidthPlotRatio()
{
	int heigth = atoi(ui.heigthPlotLE->text().toStdString().c_str());
	if (memPlot.height() != heigth)
	{
		memPlot.setHeight(heigth);
		ui.widthPlotLE->setText(QString::number(ceil(1.68865 * heigth)));
	}
}
