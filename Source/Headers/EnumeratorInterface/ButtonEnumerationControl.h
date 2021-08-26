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


#ifndef BUTTONENUMERATIONCONTROL_H
#define BUTTONENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class ButtonEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    ButtonEnumerationControl(int32_t id, QString name, QString unit, bool bIsReadOnly, QWidget *parent);

signals:
    // This signal passes info about action which needs to be performed
    //
    // Parameters:
    // [in] (int32_t) id - id of the control
    void PassActionPerform(int32_t id);

private slots:
    // This slot function is called after button is pressed
    void OnButtonClicked();

private:
    // Main element of the class, after pressing the action is going to be performed.
    QPushButton m_PushButton;
};

#endif // BUTTONENUMERATIONCONTROL_H
