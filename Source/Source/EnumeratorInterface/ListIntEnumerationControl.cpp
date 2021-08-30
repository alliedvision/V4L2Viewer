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


#include "ListIntEnumerationControl.h"

ListIntEnumerationControl::ListIntEnumerationControl(int32_t id, int32_t value, QList<int64_t> list, QString name, QString unit, bool bIsReadOnly, QWidget *parent):
    IControlEnumerationHolder(id, name, parent)
{
    m_ControlInfo = QString(tr("%1 control is represented as list of integers. \n Unit: %2")
                          .arg(name)
                          .arg(unit));

    m_MainLayout.addWidget(&m_NameOfControl, 0, 0);
    m_MainLayout.addWidget(&m_ComboBox, 0, 1);

    m_MainLayout.setColumnStretch(0, 1);
    m_MainLayout.setColumnStretch(1, 0);

    for (QList<int64_t>::iterator it = list.begin(); it<list.end(); ++it)
    {
        m_ComboBox.addItem(QString::number(*it));
    }

    if (value >= list.count())
    {
        m_ComboBox.addItem("Unknown Value");
        m_ComboBox.setCurrentText("Unknown Value");
    }
    else
    {
        m_ComboBox.setCurrentText(QString::number(list.at(value)));
    }

    if (bIsReadOnly)
    {
        setEnabled(false);
        m_ControlInfo += tr("\n Control is READONLY");
    }
    else
    {
        setEnabled(true);
        connect(&m_ComboBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnListItemChanged(const QString &)));
    }
}

void ListIntEnumerationControl::UpdateValue(QList<int64_t> list, int32_t value)
{
    m_ComboBox.blockSignals(true);
    m_ComboBox.clear();
    for (QList<int64_t>::iterator it = list.begin(); it<list.end(); ++it)
    {
        m_ComboBox.addItem(QString::number(*it));
    }

    if (value >= list.count())
    {
        m_ComboBox.addItem("Unknown Value");
        m_ComboBox.setCurrentText("Unknown Value");
    }
    else
    {
        m_ComboBox.setCurrentText(QString::number(list.at(value)));
    }

    m_ComboBox.blockSignals(false);
}

void ListIntEnumerationControl::OnListItemChanged(const QString &currentText)
{
    int64_t value = static_cast<int64_t>(currentText.toLongLong());
    emit PassNewValue(m_id, value);
}
