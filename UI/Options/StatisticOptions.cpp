#include "StatisticOptions.h"

StatisticOptions::StatisticOptions(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	connectSignals();
}

StatisticOptions::~StatisticOptions()
{

}

InsermLibrary::statOption StatisticOptions::getStatOption()
{
    InsermLibrary::statOption statOpt;
	statOpt.kruskall = ui.pCheckBoxKW->isChecked();
	statOpt.FDRkruskall = ui.FDRCheckBoxKW->isChecked();
    statOpt.pKruskall = ui.pValueLE_KW->text().toFloat();
    //===
    statOpt.wilcoxon = ui.pCheckBoxWil->isChecked();
	statOpt.FDRwilcoxon = ui.FDRCheckBoxWil->isChecked();
    statOpt.pWilcoxon = ui.pValueLE_Wil->text().toFloat();
    //===
    statOpt.BlocWiseStatistics = ui.BlocWiseStatisticsCheckBox->isChecked();
    //===
    return statOpt;
}

void StatisticOptions::connectSignals()
{
    connect(ui.pValueLE_KW, &QLineEdit::editingFinished, this, &StatisticOptions::pValueKruskall);
    connect(ui.pCheckBoxKW, &QPushButton::clicked, this, &StatisticOptions::updateKWOpt);
    connect(ui.pValueLE_Wil, &QLineEdit::editingFinished, this, &StatisticOptions::pValueWilcoxon);
    connect(ui.pCheckBoxWil, &QPushButton::clicked, this, &StatisticOptions::updateWilOpt);
    connect(ui.okPushButton, &QPushButton::clicked, this, [&]{ close(); });
}

void StatisticOptions::pValueKruskall()
{
    QLocale locale(QLocale::English, QLocale::UnitedStates);

    bool parseOk = false;
    float value = locale.toFloat(ui.pValueLE_KW->text(), &parseOk);
    if(parseOk)
    {
        if(value <= 0.0f)
        {
            ui.pValueLE_KW->setText("0.05");
        }
    }
    else //probably using , separator for decimal numbers
    {
        ui.pValueLE_KW->setText(ui.pValueLE_KW->text().replace(QString(","), QString(".")));
        float value = locale.toFloat(ui.pValueLE_KW->text(), &parseOk);
        if(value <= 0.0f)
        {
            ui.pValueLE_KW->setText("0.05");
        }
    }
}

void StatisticOptions::pValueWilcoxon()
{
    QLocale locale(QLocale::English, QLocale::UnitedStates);

    bool parseOk = false;
    float value = locale.toFloat(ui.pValueLE_Wil->text(), &parseOk);
    if(parseOk)
    {
        if(value <= 0.0f)
        {
            ui.pValueLE_Wil->setText("0.05");
        }
    }
    else //probably using , separator for decimal numbers
    {
        ui.pValueLE_Wil->setText(ui.pValueLE_Wil->text().replace(QString(","), QString(".")));
        float value = locale.toFloat(ui.pValueLE_Wil->text(), &parseOk);
        if(value <= 0.0f)
        {
            ui.pValueLE_Wil->setText("0.05");
        }
    }
}

void StatisticOptions::updateWilOpt()
{
	if (ui.pCheckBoxWil->isChecked() == false)
	{
		ui.FDRCheckBoxWil->setChecked(false);
	}

	if (ui.FDRCheckBoxWil->isChecked())
	{
		ui.pCheckBoxWil->setChecked(true);
	}
}

void StatisticOptions::updateKWOpt()
{
	if (ui.pCheckBoxKW->isChecked() == false)
	{
		ui.FDRCheckBoxKW->setChecked(false);
	}

	if (ui.FDRCheckBoxKW->isChecked())
	{
		ui.pCheckBoxKW->setChecked(true);
	}
}
