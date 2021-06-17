#include "EnumeratorInterface/IControlEnumerationHolder.h"

IControlEnumerationHolder::IControlEnumerationHolder(QString name, QWidget *parent) : QWidget(parent)
{
    m_NameOfControl.setText(name);
    m_Layout.addWidget(&m_NameOfControl, 0, 0);
    m_Layout.addWidget(&m_MainWidget, 1, 0);
    setLayout(&m_Layout);
}

IControlEnumerationHolder::~IControlEnumerationHolder()
{

}
