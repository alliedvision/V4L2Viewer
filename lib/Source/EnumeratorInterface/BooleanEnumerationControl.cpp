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


#include "BooleanEnumerationControl.h"

BooleanEnumerationControl::BooleanEnumerationControl(int32_t id, bool value, QString name, QString unit, bool bIsReadOnly, QWidget *parent):
    IControlEnumerationHolder(id, name, parent)
{
    m_ControlInfo = QString(tr("%1 control accepts boolean values. \n Unit: %2")
                          .arg(name)
                          .arg(unit));

    m_MainLayout.addWidget(&m_NameOfControl, 0, 0);
    m_MainLayout.addWidget(&m_ComboBox, 0, 1);
    m_MainLayout.setColumnStretch(0, 1);
    m_MainLayout.setColumnStretch(1, 0);

    m_ComboBox.addItem("True");
    m_ComboBox.addItem("False");

    m_ComboBox.setCurrentText(value ? "True" : "False");

    if (bIsReadOnly)
    {
        setEnabled(false);
        m_ControlInfo += tr("\n Control is READONLY");
    }
    else
    {
        setEnabled(true);
        connect(&m_ComboBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnTextChanged(const QString &)));
    }
}

void BooleanEnumerationControl::UpdateValue(bool val)
{
    m_ComboBox.blockSignals(true);
    m_ComboBox.setCurrentText(val ? "True" : "False");
    m_ComboBox.blockSignals(false);
}

void BooleanEnumerationControl::OnTextChanged(const QString &text)
{
    if (text == "False")
    {
        emit PassNewValue(m_id, false);
    }
    else
    {
        emit PassNewValue(m_id, true);
    }
}

