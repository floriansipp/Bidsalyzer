#include "FrequencyWindow.h"

FrequencyWindow::FrequencyWindow(InsermLibrary::FrequencyBand& f, QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.OkCancelButtonBox, &QDialogButtonBox::accepted, this, &FrequencyWindow::ValidateAndSave);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::rejected, this, [&] { close(); });

    m_frequencyBand = &f;

    ui.NameLineEdit->setText(m_frequencyBand->Label().c_str());
    ui.MinimumFrequencyLineEdit->setText(QString::number(m_frequencyBand->FMin()));
    ui.StepFrequencyLineEdit->setText(QString::number(m_frequencyBand->Step()));
    ui.MaximumFrequencyLineEdit->setText(QString::number(m_frequencyBand->FMax()));
}

FrequencyWindow::~FrequencyWindow()
{

}

void FrequencyWindow::ValidateAndSave()
{
    //modif data in the struct from element in ui
    std::string label = ui.NameLineEdit->text().toStdString();
    int fMin = ui.MinimumFrequencyLineEdit->text().toInt();
    int step = ui.StepFrequencyLineEdit->text().toInt();
    int fMax = ui.MaximumFrequencyLineEdit->text().toInt();

    m_frequencyBand->Label(label);
    m_frequencyBand->UpdateFrequencies(fMin, fMax, step);

    done(1);
    close();
}
