#include "SubBlocWindow.h"
#include "EventWindow.h"
#include "IconWindow.h"

SubBlocWindow::SubBlocWindow(InsermLibrary::SubBloc& subbloc, QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    SetupComboBoxType();

    //Event related
    connect(ui.AddEventPushButton, &QPushButton::clicked, this, &SubBlocWindow::AddEventElement);
    connect(ui.RemoveEventPushButton, &QPushButton::clicked, this, &SubBlocWindow::RemoveEventElement);
    connect(ui.EventsListWidget, &QListWidget::itemDoubleClicked, this, &SubBlocWindow::OnEventDoubleClicked);
    //Icon related
    connect(ui.AddIconsPushButton, &QPushButton::clicked, this, &SubBlocWindow::AddIconElement);
    connect(ui.RemoveIconsPushButton, &QPushButton::clicked, this, &SubBlocWindow::RemoveIconElement);
    connect(ui.IconsListWidget, &QListWidget::itemDoubleClicked, this, &SubBlocWindow::OnIconDoubleClicked);
    //===
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::accepted, this, &SubBlocWindow::ValidateModifications);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::rejected, this, [&] { close(); });

    m_subbloc = &subbloc;

    LoadInUi();
    LoadEvents();
    LoadIcons();
}

SubBlocWindow::~SubBlocWindow()
{

}

void SubBlocWindow::SetupComboBoxType()
{
    QMap<QString, InsermLibrary::MainSecondaryEnum>* flowControlOptions = new QMap<QString, InsermLibrary::MainSecondaryEnum>;
    flowControlOptions->insert("Main", InsermLibrary::MainSecondaryEnum::Main);
    flowControlOptions->insert("Secondary", InsermLibrary::MainSecondaryEnum::Secondary);
    ui.TypeComboBox->addItems(QStringList(flowControlOptions->keys()));
}

void SubBlocWindow::LoadInUi()
{
    ui.SubBlocNameLineEdit->setText(m_subbloc->Name().c_str());
    ui.OrderLineEdit->setText(QString::number(m_subbloc->Order()));
    int typeIndex = (m_subbloc->Type() == InsermLibrary::MainSecondaryEnum::Main) ? 0 : 1;
    ui.TypeComboBox->setCurrentIndex(typeIndex);
    ui.WindowStartLineEdit->setText(QString::number(m_subbloc->MainWindow().Start()));
    ui.WindowEndLineEdit->setText(QString::number(m_subbloc->MainWindow().End()));
    ui.BaselineStartLineEdit->setText(QString::number(m_subbloc->Baseline().Start()));
    ui.BaselineEndLineEdit->setText(QString::number(m_subbloc->Baseline().End()));
}

void SubBlocWindow::UpdateEventDisplay(int index, std::string name)
{
    ui.EventsListWidget->item(index)->setText(name.c_str());
}

void SubBlocWindow::UpdateIconDisplay(int index, std::string name)
{
    ui.IconsListWidget->item(index)->setText(name.c_str());
}

void SubBlocWindow::LoadEvents()
{
    for (int i = 0; i < m_subbloc->Events().size(); i++)
    {
        QListWidgetItem* currentPROV = new QListWidgetItem(ui.EventsListWidget);
        currentPROV->setText(m_subbloc->Events()[i].Name().c_str());
        currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsSelectable);
    }
}

void SubBlocWindow::LoadIcons()
{
    for (int i = 0; i < m_subbloc->Icons().size(); i++)
    {
        QListWidgetItem* currentPROV = new QListWidgetItem(ui.IconsListWidget);
        currentPROV->setText(m_subbloc->Icons()[i].Name().c_str());
        currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsSelectable);
    }
}

void SubBlocWindow::OnEventDoubleClicked()
{
    QModelIndexList indexes = ui.EventsListWidget->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty())
    {
        m_IndexOfEvent = indexes[0].row();

        m_memoryEvent = InsermLibrary::Event(m_subbloc->Events()[m_IndexOfEvent]);
        EventWindow* blocWindow = new EventWindow(m_subbloc->Events()[m_IndexOfEvent], this);
        blocWindow->setAttribute(Qt::WA_DeleteOnClose);
        blocWindow->show();

        connect(blocWindow, &EventWindow::accepted, this, &SubBlocWindow::OnEventWindowAccepted);
        connect(blocWindow, &EventWindow::rejected, this, &SubBlocWindow::OnEventWindowRejected);
    }
}

void SubBlocWindow::AddEventElement()
{
    QModelIndexList indexes = ui.EventsListWidget->selectionModel()->selectedIndexes();
    int insertionIndex = !indexes.isEmpty() ? indexes[0].row() + 1 : ui.EventsListWidget->count();
    m_subbloc->Events().insert(m_subbloc->Events().begin() + insertionIndex, InsermLibrary::Event());
    ui.EventsListWidget->insertItem(insertionIndex, m_subbloc->Events()[insertionIndex].Name().c_str());
}

void SubBlocWindow::RemoveEventElement()
{
    QModelIndexList indexes = ui.EventsListWidget->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty())
    {
        int indexToDelete = indexes[0].row();
        ui.EventsListWidget->item(indexToDelete)->~QListWidgetItem();
        m_subbloc->Events().erase(m_subbloc->Events().begin() + indexToDelete);
    }
}

void SubBlocWindow::OnEventWindowAccepted()
{
    std::cout << "OnEventWindowAccepted" << std::endl;
    UpdateEventDisplay(m_IndexOfEvent, m_subbloc->Events()[m_IndexOfEvent].Name());
}

void SubBlocWindow::OnEventWindowRejected()
{
    std::cout << "OnEventWindowRejected" << std::endl;
    m_subbloc->Events()[m_IndexOfEvent] = InsermLibrary::Event(m_memoryEvent);
}

void SubBlocWindow::OnIconDoubleClicked()
{
    QModelIndexList indexes = ui.IconsListWidget->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty())
    {
        m_IndexOfIcon = indexes[0].row();

        m_memoryIcon = InsermLibrary::Icon(m_subbloc->Icons()[m_IndexOfIcon]);
        IconWindow* blocWindow = new IconWindow(m_subbloc->Icons()[m_IndexOfIcon], this);
        blocWindow->setAttribute(Qt::WA_DeleteOnClose);
        blocWindow->show();

        connect(blocWindow, &IconWindow::accepted, this, &SubBlocWindow::OnIconWindowAccepted);
        connect(blocWindow, &IconWindow::rejected, this, &SubBlocWindow::OnIconWindowRejected);
    }
}

void SubBlocWindow::AddIconElement()
{
    QModelIndexList indexes = ui.IconsListWidget->selectionModel()->selectedIndexes();
    int insertionIndex = !indexes.isEmpty() ? indexes[0].row() + 1 : ui.IconsListWidget->count();
    m_subbloc->Icons().insert(m_subbloc->Icons().begin() + insertionIndex, InsermLibrary::Icon());
    ui.IconsListWidget->insertItem(insertionIndex, m_subbloc->Icons()[insertionIndex].Name().c_str());
}

void SubBlocWindow::RemoveIconElement()
{
    QModelIndexList indexes = ui.IconsListWidget->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty())
    {
        int indexToDelete = indexes[0].row();
        ui.IconsListWidget->item(indexToDelete)->~QListWidgetItem();
        m_subbloc->Icons().erase(m_subbloc->Icons().begin() + indexToDelete);
    }
}

void SubBlocWindow::OnIconWindowAccepted()
{
    std::cout << "OnIconWindowAccepted" << std::endl;
    UpdateIconDisplay(m_IndexOfIcon, m_subbloc->Icons()[m_IndexOfIcon].Name());
}

void SubBlocWindow::OnIconWindowRejected()
{
    std::cout << "OnIconWindowRejected" << std::endl;
    m_subbloc->Icons()[m_IndexOfIcon] = InsermLibrary::Icon(m_memoryIcon);
}

void SubBlocWindow::ValidateModifications()
{    
    m_subbloc->Name(ui.SubBlocNameLineEdit->text().toStdString());
    m_subbloc->Order(ui.OrderLineEdit->text().toInt());
    InsermLibrary::MainSecondaryEnum type = ui.TypeComboBox->currentIndex() == 0 ? InsermLibrary::MainSecondaryEnum::Main : InsermLibrary::MainSecondaryEnum::Secondary;
    m_subbloc->Type(type);
    InsermLibrary::Window mainwin(ui.WindowStartLineEdit->text().toInt(), ui.WindowEndLineEdit->text().toInt());
    m_subbloc->MainWindow(mainwin);
    InsermLibrary::Window baselinewin(ui.BaselineStartLineEdit->text().toInt(), ui.BaselineEndLineEdit->text().toInt());
    m_subbloc->Baseline(baselinewin);
    done(1);
    close();
}
