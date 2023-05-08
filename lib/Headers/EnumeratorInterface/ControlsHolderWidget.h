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


#ifndef CONTROLSHOLDERWIDGET_H
#define CONTROLSHOLDERWIDGET_H

#include <QDialog>
#include <QScrollArea>
#include <QGridLayout>
#include <EnumeratorInterface/IControlEnumerationHolder.h>
#include "ui_ControlsHolderWidget.h"

class ControlsHolderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControlsHolderWidget(QWidget *parent = nullptr);
    // This function add new enumeration control to the list
    //
    // Parameters:
    // [in] (IControlEnumerationHolder *) controlWidget - element which is dynamically casted to the specific control
    void AddElement(IControlEnumerationHolder *controlWidget);
    // This function removes all of the elements from the list, for example when camera is closed
    void RemoveElements();
    // This function returns bool state which depends on whether control is already set or not. This is useful,
    // when updating some of the values
    //
    // Parameters:
    // [in] (int32_t) id - control id to be checked
    //
    // Returns:
    // (bool) - boolean value
    bool IsControlAlreadySet(int32_t id);
    // This function returns IControlEnumerationHolder* depending on the passed id value
    //
    // Parameters:
    // [in] (int32_t) id - control id value
    // [in] (bool &) bIsSuccess - boolean value which ensures about found item
    //
    // Returns:
    // (IControlEnumerationHolder*) - found object
    IControlEnumerationHolder* GetControlWidget(int32_t id, bool &bIsSuccess);

    virtual QSize sizeHint() const override;

    Ui::ControlsHolderWidget ui;

private slots:
    // This slot function is called when the element on the list changed
    //
    // Parameters:
    // [in] row - current row
    void OnListItemChanged(int row);

private:
    // This vector stores all of the controls widgets
    QVector<IControlEnumerationHolder*> m_itemVector;
};


#endif // CONTROLSHOLDERWIDGET_H
