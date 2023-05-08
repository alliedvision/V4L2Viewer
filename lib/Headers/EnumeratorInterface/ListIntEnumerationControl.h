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


#ifndef LISTINTENUMERATIONCONTROL_H
#define LISTINTENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class ListIntEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    ListIntEnumerationControl(int32_t id, int32_t value, QList<int64_t> list, QString name, QString unit, bool bIsReadOnly, QWidget *parent);
    // This function updates value in current widget when it's changed somewhere else
    //
    // Parameters:
    // [in] (QList<int64_t>) list - new list values for the widget
    // [in] (int32_t) value - new value for the widget
    void UpdateValue(QList<int64_t> list, int32_t value);

private slots:
    // This slot function is called when the item on the ListWidget was changed
    //
    // Parameters:
    // [in] (const QString &) currentText - string value which is passed to the function from the list widget
    void OnListItemChanged(const QString &currentText);

signals:
    // This signal passes new string value to the Camera class
    //
    // Parameters:
    // [in] (int32_t) id - id of the current control
    // [in] (int64_t) value - new int64 value to be passed
    void PassNewValue(int32_t id, int64_t value);

private:
    // Main element of the class
    QComboBox m_ComboBox;
};

#endif // LISTINTENUMERATIONCONTROL_H
