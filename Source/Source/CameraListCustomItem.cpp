#include "CameraListCustomItem.h"

CameraListCustomItem::CameraListCustomItem(QString cameraName, QWidget *parent) :
    QWidget(parent),
    m_CameraName(cameraName),
    m_CameraDesc("")
{
    m_CameraDesc.setTextFormat(Qt::RichText);
    m_CameraName.setTextFormat(Qt::RichText);
    m_CameraDesc.setStyleSheet("background:transparent");
    m_CameraName.setStyleSheet("background:transparent");
    m_CameraDesc.setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_pMainLayout = new QGridLayout(this);
    m_pMainLayout->addWidget(&m_CameraName, 0, 0);
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
