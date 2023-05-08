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


#ifndef ICONTROLENUMERATIONHOLDER_H
#define ICONTROLENUMERATIONHOLDER_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>

class IControlEnumerationHolder : public QWidget
{
    Q_OBJECT
public:
    explicit IControlEnumerationHolder(int32_t id, QString name, QWidget *parent = nullptr);
    virtual ~IControlEnumerationHolder() = 0;
    // This function returns the id of the control
    //
    // Returns:
    // (int32_t) - id value
    int32_t GetWidgetControlId();
    // This function returns info about control, which then is displayed in ControlsHolderWidget
    //
    // Returns:
    // (QString) - control info string
    QString GetControlInfo();

protected:
    QLabel              m_NameOfControl;
    QString             m_ControlInfo;
    QLabel              m_CurrentValue;
    QGridLayout         m_MainLayout;
    int32_t             m_id;
};

#endif // ICONTROLENUMERATIONHOLDER_H
