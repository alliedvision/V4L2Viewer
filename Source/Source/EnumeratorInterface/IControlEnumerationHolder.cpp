#include "IControlEnumerationHolder.h"

IControlEnumerationHolder::IControlEnumerationHolder(int32_t id, QString name, QWidget *parent) : m_id(id), QWidget(parent)
{
    m_NameOfControl.setText(name);
    m_NameOfControl.setWordWrap(true);
    m_Layout.addWidget(&m_NameOfControl, 0, 0);
    m_Layout.addWidget(&m_MainWidget, 1, 0);
    setLayout(&m_Layout);
}

int32_t IControlEnumerationHolder::GetWidgetControlId()
{
    return m_id;
}

IControlEnumerationHolder::~IControlEnumerationHolder()
{

}
