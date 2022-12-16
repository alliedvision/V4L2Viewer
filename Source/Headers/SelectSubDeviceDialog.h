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


#ifndef SELECTSUBDEVICEDIALOG_H
#define SELECTSUBDEVICEDIALOG_H

#include <QObject>
#include <QVector>
#include <QtWidgets>
#include <QDialog>
#include <QString>
#include <QCheckBox>

class SelectSubDeviceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectSubDeviceDialog(QVector<QString> const& subDeviceList, QVector<QString>& selectedSubDeviceList, QWidget *parent = nullptr);

private slots:
    void OnAccepted();

private:
    QVector<QString> const& m_subDeviceList;
    QVector<QString>& m_selectedSubDeviceList;
    QLabel *label;
    QVector<QCheckBox*> m_subDeviceCheckBoxes;
};

#endif // SELECTSUBDEVICEDIALOG_H
