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


#include "ButtonEnumerationControl.h"

ButtonEnumerationControl::ButtonEnumerationControl(int32_t id, QString name, QString unit, bool bIsReadOnly, QWidget *parent) :
    IControlEnumerationHolder(id, name, parent)
{
    m_ControlInfo = QString(tr("%1 control performs action after clicking button below. \n Unit: %2")
                          .arg(name)
                          .arg(unit));
    m_PushButton.setText("Perform");

    m_MainLayout.addWidget(&m_NameOfControl, 0, 0);
    m_MainLayout.addWidget(&m_PushButton, 0, 1);

    m_MainLayout.setColumnStretch(0, 1);
    m_MainLayout.setColumnStretch(1, 0);

    if (bIsReadOnly)
    {
        setEnabled(false);
    }
    else
    {
        setEnabled(true);
    }

	connect(&m_PushButton, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
}

void ButtonEnumerationControl::OnButtonClicked()
{
    emit PassActionPerform(m_id);
}
