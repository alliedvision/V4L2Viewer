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


#include "CameraListCustomItem.h"

CameraListCustomItem::CameraListCustomItem(QString cameraName, QWidget *parent) :
    QWidget(parent),
    m_CameraName(cameraName),
    m_CameraDesc("")
{
    m_CameraName.setTextFormat(Qt::RichText);
    m_CameraName.setStyleSheet("background:transparent");

    m_CameraDesc.setTextFormat(Qt::RichText);
    m_CameraDesc.setStyleSheet("background:transparent");
    m_CameraDesc.setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_pMainLayout = new QGridLayout(this);
    m_pMainLayout->addWidget(&m_CameraName, 0, 0);

    m_pMainLayout->setMargin(0);
    setLayout(m_pMainLayout);
}

void CameraListCustomItem::SetCameraInformation(QString cameraDesc)
{
    m_CameraDesc.setText(cameraDesc);
    m_pMainLayout->addWidget(&m_CameraDesc, 1, 0);
}

void CameraListCustomItem::RemoveCameraInformation()
{
    m_pMainLayout->removeWidget(&m_CameraDesc);
}

QString CameraListCustomItem::GetCameraName()
{
    return m_CameraName.text();
}
