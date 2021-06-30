#include "IControlEnumerationHolder.h"
#include <QMouseEvent>

IControlEnumerationHolder::IControlEnumerationHolder(int32_t id, QString name, QWidget *parent) :
    QWidget(parent),
    m_id(id)
{
    m_NameOfControl.setText(name);
    m_MainLayout.addWidget(&m_NameOfControl, 0, 0);
    m_MainLayout.addWidget(&m_CurrentValue, 0, 1);
    setLayout(&m_MainLayout);
    m_MainLayout.setMargin(0);
}

int32_t IControlEnumerationHolder::GetWidgetControlId()
{
    return m_id;
}

QString IControlEnumerationHolder::GetControlInfo()
{
    return m_ControlInfo;
}

void IControlEnumerationHolder::mouseDoubleClickEvent(QMouseEvent *event)
{
    QPoint posGlobal = mapToGlobal(event->pos());
    m_ControlEditWidget.setGeometry(posGlobal.x(), posGlobal.y(), this->width(), this->height());
    m_ControlEditWidget.show();
}

IControlEnumerationHolder::~IControlEnumerationHolder()
{

}
