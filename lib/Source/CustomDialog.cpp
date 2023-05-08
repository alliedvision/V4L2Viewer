/* Allied Vision V4L2Viewer - Graphical Video4Linux Viewer Example
   Copyright (C) 2021 Allied Vision Technologies GmbH

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.  */

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
