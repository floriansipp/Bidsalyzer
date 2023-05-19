#include "IconWindow.h"

IconWindow::IconWindow(InsermLibrary::Icon& sbicon, QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    m_Icon = &sbicon;
    m_imagePath = m_Icon->Path();
    //===
    connect(ui.BrowseIconButton, &QPushButton::clicked, this, &::IconWindow::BrowseIcon);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::accepted, this, &IconWindow::ValidateModifications);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::rejected, this, [&] { close(); });
    //===
    ui.IconNameLineEdit->setText(m_Icon->Name().c_str());
    ui.WindowStartLineEdit->setText(QString::number(m_Icon->DisplayWindow().Start()));
    ui.WindowEndLineEdit->setText(QString::number(m_Icon->DisplayWindow().End()));
    ui.IconLabel->setPixmap(QPixmap(m_imagePath.c_str()));
}

IconWindow::~IconWindow()
{

}

void IconWindow::BrowseIcon()
{
    QFileDialog *fileDial = new QFileDialog(this);
    fileDial->setFileMode(QFileDialog::AnyFile);
    fileDial->setNameFilters(QStringList()<<"*.jpg"<<" *.png"<<" *.bmp");
    QString fileName = fileDial->getOpenFileName(this, tr("Choose Icon Picture"));
    if (fileName != "" && QFileInfo::exists(fileName))
    {
        m_imagePath = fileName.toStdString();
        ui.IconLabel->setPixmap(QPixmap(m_imagePath.c_str()));
    }
}

void IconWindow::ValidateModifications()
{
    m_Icon->Name(ui.IconNameLineEdit->text().toStdString());
    m_Icon->DisplayWindow().Start(ui.WindowStartLineEdit->text().toInt());
    m_Icon->DisplayWindow().End(ui.WindowEndLineEdit->text().toInt());
    m_Icon->Path(m_imagePath);

    done(1);
    close();
}
