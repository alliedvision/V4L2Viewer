#include "IControlEnumerationHolder.h"

IControlEnumerationHolder::IControlEnumerationHolder(int32_t id, QString name, QWidget *parent) : m_id(id), QWidget(parent)
{
    m_NameOfControl.setText(name);
    m_ControlWidget.setLayout(&m_ControlWidgetLayout);
    m_MainLayout.addWidget(&m_NameOfControl, 0, 0);
    m_MainLayout.addWidget(&m_ControlInfo, 1, 0);
    m_MainLayout.addWidget(&m_ControlWidget, 2, 0);
    setLayout(&m_MainLayout);
}

int32_t IControlEnumerationHolder::GetWidgetControlId()
{
    return m_id;
}

IControlEnumerationHolder::~IControlEnumerationHolder()
{

}
