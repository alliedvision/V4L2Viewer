#ifndef ICONTROLENUMERATIONHOLDER_H
#define ICONTROLENUMERATIONHOLDER_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QCheckBox>
#include <QPushButton>
#include "ControlEditWidget.h"

class IControlEnumerationHolder : public QWidget
{
    Q_OBJECT
public:
    explicit IControlEnumerationHolder(int32_t id, QString name, QWidget *parent = nullptr);
    virtual ~IControlEnumerationHolder() = 0;
    int32_t GetWidgetControlId();
    QString GetControlInfo();
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

protected:
    QLabel              m_NameOfControl;
    QString             m_ControlInfo;
    QLabel              m_CurrentValue;
    QGridLayout         m_MainLayout;
    ControlEditWidget   m_ControlEditWidget;
    int32_t             m_id;
};

#endif // ICONTROLENUMERATIONHOLDER_H
