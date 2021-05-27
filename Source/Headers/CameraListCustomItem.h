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
    void SetCameraInformation(QString cameraDesc);
    QString GetCameraName();

private:
    QGridLayout *m_pMainLayout;
    QLabel m_CameraName;
    QLabel m_CameraDesc;
};

#endif // CAMERALISTCUSTOMITEM_H
