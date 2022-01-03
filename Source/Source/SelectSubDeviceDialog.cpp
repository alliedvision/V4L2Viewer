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

#include "SelectSubDeviceDialog.h"
#include "Logger.h"

#include <fstream>

SelectSubDeviceDialog::SelectSubDeviceDialog(QVector<QString> const& subDeviceList, QVector<QString>& selectedSubDeviceList, QWidget *parent)
: m_subDeviceList(subDeviceList),
  m_selectedSubDeviceList(selectedSubDeviceList),
  QDialog(parent)
{
    setModal(true);

    QLabel* prompt = new QLabel("Please select any sub-devices:");

    {
    QPalette palette = prompt->palette();
    palette.setColor(QPalette::Active, QPalette::WindowText, Qt::white);
    prompt->setPalette(palette);
    }

    QWidget* selection = new QWidget;

    QVBoxLayout *selectionLayout = new QVBoxLayout;
    selectionLayout->setContentsMargins(QMargins());
    for(auto const& subDevice : m_subDeviceList)
    {
        std::string name;
        std::string subDeviceNoDev = subDevice.toStdString().substr(5);
        std::string filePath("/sys/class/video4linux/" + subDeviceNoDev + "/name");
        std::ifstream nameStream(filePath.c_str());
        std::getline(nameStream, name);

        QString checkBoxText = subDevice;
        if(!name.empty())
        {
            checkBoxText += QString(" (%1)").arg(name.c_str());
        }
        QCheckBox* checkBox = new QCheckBox(checkBoxText);
        checkBox->setStyleSheet("background-color: white;");
        m_subDeviceCheckBoxes.push_back(checkBox);
        selectionLayout->addWidget(checkBox);
    }

    selection->setLayout(selectionLayout);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(prompt, 0, 0);
    mainLayout->addWidget(selection, 1, 0);
    mainLayout->addWidget(buttonBox, 2, 0);
    mainLayout->setRowStretch(1, 1);

    setLayout(mainLayout);

    setWindowTitle(tr("Sub-devices"));

    connect(this, SIGNAL(accepted()), this, SLOT(OnAccepted()));
}

void SelectSubDeviceDialog::OnAccepted()
{
    QVector<QString>::const_iterator itSubDeviceList = m_subDeviceList.begin();
    for(auto* checkBox : m_subDeviceCheckBoxes)
    {
        if(checkBox->isChecked())
        {
            m_selectedSubDeviceList.push_back(*itSubDeviceList);
        }
        ++itSubDeviceList;
    }
    close();
}
