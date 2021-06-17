#include "EnumeratorInterface/IntegerEnumerationControl.h"

IntegerEnumerationControl::IntegerEnumerationControl(QString name, QWidget *parent): IControlEnumerationHolder(name, parent)
{
    m_ControlInfo.setText("Control info here");
    m_Layout.addWidget(&m_ControlInfo, 0, 0);
    m_Layout.addWidget(&m_LineEdit, 1, 0);
    m_MainWidget.setLayout(&m_Layout);
}

QLineEdit &IntegerEnumerationControl::GetLineEdit()
{
    return m_LineEdit;
}
