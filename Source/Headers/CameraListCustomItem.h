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


#ifndef CAMERALISTCUSTOMITEM_H
#define CAMERALISTCUSTOMITEM_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>

class CameraListCustomItem : public QWidget
{
    Q_OBJECT
public:
    explicit CameraListCustomItem(QString cameraName, QWidget *parent = nullptr);
    // This function sets all necessary camera informations to the widget
    //
    // Parameters:
    // [in] (QString) cameraDesc - string variable which stores all data
    void SetCameraInformation(QString cameraDesc);
    // This function removes camera information from the widget, when camera is closed
    void RemoveCameraInformation();
    // This function returns camera name
    //
    // Returns:
    // (QString) - camera name
    QString GetCameraName();

private:
    QGridLayout *m_pMainLayout;
    QLabel m_CameraName;
    QLabel m_CameraDesc;
};

#endif // CAMERALISTCUSTOMITEM_H
