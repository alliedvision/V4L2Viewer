#include "CustomDialog.h"

CustomDialog::CustomDialog(QString title, QString text, int type, QWidget *parent)
    : QDialog(parent)
    , m_Text(text)
    , m_Title(title)
{
    ui.setupUi(this);

    switch(type)
    {
        case CustomDialog::warning: ui.m_Title->setText("WARNING"); break;
        case CustomDialog::info: ui.m_Title->setText("INFORMATION");  break;
        case CustomDialog::error: ui.m_Title->setText("ERROR"); break;
    }
    this->setWindowTitle(m_Title);
    ui.m_Content->setText(text);

    connect(ui.m_OkButton, SIGNAL(clicked()), this, SLOT(OnOkButtonClicked()));
}

void CustomDialog::closeEvent(QCloseEvent *event)
{
    this->close();
    this->deleteLater();
    QWidget::closeEvent(event);
}

void CustomDialog::OnOkButtonClicked()
{
    this->close();
}

void CustomDialog::Warning(QWidget *parent, QString title, QString text)
{
    CustomDialog *dialog = new CustomDialog(title, text, CustomDialog::warning, parent);
    dialog->show();
}

void CustomDialog::Info(QWidget *parent, QString title, QString text)
{
    CustomDialog *dialog = new CustomDialog(title, text, CustomDialog::info, parent);
    dialog->show();
}

void CustomDialog::Error(QWidget *parent, QString title, QString text)
{
    CustomDialog *dialog = new CustomDialog(title, text, CustomDialog::error, parent);
    dialog->show();
}
