#include "FrequenciesWindow.h"
#include "FrequencyWindow.h"

FrequenciesWindow::FrequenciesWindow(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.FrequenciesListWidget, &QListWidget::itemDoubleClicked, this, &FrequenciesWindow::OnFrequencyDoubleClicked);
    connect(ui.AddPushButton, &QPushButton::clicked, this, &FrequenciesWindow::AddElement);
    connect(ui.DeletePushButton, &QPushButton::clicked, this, &FrequenciesWindow::RemoveElement);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::accepted, this, &FrequenciesWindow::ValidateModifications);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::rejected, this, [&] { close(); });

    f = new InsermLibrary::FrequencyFile();
    f->Load();

    LoadFrequencyBands(f->FrequencyBands());
}

FrequenciesWindow::~FrequenciesWindow()
{

}

void FrequenciesWindow::LoadFrequencyBands(const std::vector<InsermLibrary::FrequencyBand>& FrequencyBands)
{
    ui.FrequenciesListWidget->clear();
    for (size_t i = 0; i < FrequencyBands.size(); i++)
    {
        QString Label = QString::fromStdString(FrequencyBands[i].Label());

        QListWidgetItem *currentBand = new QListWidgetItem(ui.FrequenciesListWidget);
        currentBand->setText(Label);
    }
}

void FrequenciesWindow::OnFrequencyDoubleClicked(QListWidgetItem* item)
{
    QModelIndexList indexes = item->listWidget()->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty())
    {
        m_FrequencyIndex = indexes[0].row();

        m_MemoryFrequency = InsermLibrary::FrequencyBand(f->FrequencyBands()[m_FrequencyIndex]);
        FrequencyWindow* protocolsWindow = new FrequencyWindow(f->FrequencyBands()[m_FrequencyIndex], this);
        protocolsWindow->setAttribute(Qt::WA_DeleteOnClose);
        protocolsWindow->show();

        connect(protocolsWindow, &FrequencyWindow::accepted, this, &FrequenciesWindow::OnFrequencyWindowAccepted);
        connect(protocolsWindow, &FrequencyWindow::rejected, this, &FrequenciesWindow::OnFrequencyWindowRejected);
    }
}

void FrequenciesWindow::AddElement()
{
    QModelIndexList indexes = ui.FrequenciesListWidget->selectionModel()->selectedIndexes();
    int insertionIndex = !indexes.isEmpty() ? indexes[0].row() + 1 : ui.FrequenciesListWidget->count();
    f->FrequencyBands().insert(f->FrequencyBands().begin() + insertionIndex, InsermLibrary::FrequencyBand());
    ui.FrequenciesListWidget->insertItem(insertionIndex, f->FrequencyBands()[insertionIndex].Label().c_str());
}

void FrequenciesWindow::RemoveElement()
{
    QModelIndexList indexes = ui.FrequenciesListWidget->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty())
    {
        int indexToDelete = indexes[0].row();
        ui.FrequenciesListWidget->item(indexToDelete)->~QListWidgetItem();
        f->FrequencyBands().erase(f->FrequencyBands().begin() + indexToDelete);
    }
}

void FrequenciesWindow::OnFrequencyWindowAccepted()
{
    std::cout << "OnFrequencyWindowAccepted" << std::endl;
    ui.FrequenciesListWidget->item(m_FrequencyIndex)->setText(f->FrequencyBands()[m_FrequencyIndex].Label().c_str());
}

void FrequenciesWindow::OnFrequencyWindowRejected()
{
    std::cout << "OnFrequencyWindowRejected" << std::endl;
    f->FrequencyBands()[m_FrequencyIndex] = InsermLibrary::FrequencyBand(m_MemoryFrequency);
}

void FrequenciesWindow::ValidateModifications()
{
    f->Save();

    done(1);
    close();
}
