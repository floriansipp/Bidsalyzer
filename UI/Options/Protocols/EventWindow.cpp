#include "EventWindow.h"

EventWindow::EventWindow(InsermLibrary::Event& sbevent, QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    SetupComboBoxType();
    m_Event = &sbevent;
    //===
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::accepted, this, &EventWindow::ValidateModifications);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::rejected, this, [&] { close(); });
    //===
    ui.EventNameLineEdit->setText(m_Event->Name().c_str());
    //==
    int typeIndex = 0;
    if (m_Event->Type() == InsermLibrary::MainSecondaryEnum::Main)
    {
        typeIndex = 0;
    }
    else if (m_Event->Type() == InsermLibrary::MainSecondaryEnum::Secondary)
    {
        typeIndex = 1;
    }
    ui.TypeComboBox->setCurrentIndex(typeIndex);
    //==
    QString codes;
    for (int i = 0; i < static_cast<int>(m_Event->Codes().size()); i++)
    {
        codes.append(QString::number(m_Event->Codes()[i]));
        if (i + 1 < static_cast<int>(m_Event->Codes().size()))
        {
            codes.append(',');
        }
    }
    ui.CodesLineEdit->setText(codes);
}

EventWindow::~EventWindow()
{

}

void EventWindow::SetupComboBoxType()
{
    QMap<QString, InsermLibrary::MainSecondaryEnum>* flowControlOptions = new QMap<QString, InsermLibrary::MainSecondaryEnum>;
    flowControlOptions->insert("Main", InsermLibrary::MainSecondaryEnum::Main);
    flowControlOptions->insert("Secondary", InsermLibrary::MainSecondaryEnum::Secondary);
    ui.TypeComboBox->addItems(QStringList(flowControlOptions->keys()));
}

void EventWindow::ValidateModifications()
{
    std::string name = ui.EventNameLineEdit->text().toStdString();

    InsermLibrary::MainSecondaryEnum type = ui.TypeComboBox->currentIndex() == 0 ? InsermLibrary::MainSecondaryEnum::Main : InsermLibrary::MainSecondaryEnum::Secondary;

    std::vector<int> codes;
    QStringList codesListString = ui.CodesLineEdit->text().split(',', Qt::SplitBehaviorFlags::SkipEmptyParts);
    for (int i = 0; i < codesListString.size(); i++)
    {
        codes.push_back(codesListString[i].toInt());
    }

    m_Event->Name(name);
    m_Event->Type(type);
    m_Event->Codes(codes);

    done(1);
    close();
}
