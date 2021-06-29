#include "IControlEnumerationHolder.h"

IControlEnumerationHolder::IControlEnumerationHolder(int32_t id, QString name, QWidget *parent) : m_id(id), QWidget(parent)
{
    m_NameOfControl.setText(name);
    m_MainLayout.addWidget(&m_NameOfControl, 0, 0);
    m_MainLayout.addWidget(&m_CurrentValue, 0, 1);
    setLayout(&m_MainLayout);
}

int32_t IControlEnumerationHolder::GetWidgetControlId()
{
    return m_id;
}

IControlEnumerationHolder::~IControlEnumerationHolder()
{

}
