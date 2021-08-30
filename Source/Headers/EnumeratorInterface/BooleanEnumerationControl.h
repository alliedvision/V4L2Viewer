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


#ifndef BOOLEANENUMERATIONCONTROL_H
#define BOOLEANENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class BooleanEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    BooleanEnumerationControl(int32_t id, bool value, QString name, QString unit, bool bIsReadOnly, QWidget *parent);
    // This function updates value in current widget when it's changed somewhere else
    //
    // Parameters:
    // [in] (bool) val - new value for the control
    void UpdateValue(bool val);

signals:
    // This signal passes new value to the Camera class
    //
    // Parameters:
    // [in] (int32_t) id - id of the control
    // [in] (bool) value - new value to be passed
    void PassNewValue(int32_t id, bool value);

private slots:
    // This slot function is called when the item in the comboBox is changed
    //
    // Parameters:
    // [in] (const QString &) text - text from the comboBox
    void OnTextChanged(const QString &text);

private:
    QComboBox m_ComboBox;
};

#endif // BOOLEANENUMERATIONCONTROL_H
