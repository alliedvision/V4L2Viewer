/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        CameraListCustomItem.cpp

  Description: This widget class stores and displays all necessary data
               about current opened camera.

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

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
