/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        CameraListCustomItem.h

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
