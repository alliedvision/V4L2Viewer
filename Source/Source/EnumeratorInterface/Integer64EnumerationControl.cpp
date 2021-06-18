#include "Integer64EnumerationControl.h"

Integer64EnumerationControl::Integer64EnumerationControl(int64_t min, int64_t max, int64_t step, int64_t value, QString name, QWidget *parent):
    IControlEnumerationHolder(name, parent),
    m_Min(min),
    m_Max(max),
    m_Step(step),
    m_Value(value)
{
    m_ControlInfo.setText("Control info here");
    m_Layout.addWidget(&m_ControlInfo, 0, 0);
    m_Layout.addWidget(&m_LineEdit, 1, 0);
    m_MainWidget.setLayout(&m_Layout);
    connect(&m_LineEdit, SIGNAL(returnPressed()), this, SLOT(OnLineEditPressed()));
}

void Integer64EnumerationControl::OnLineEditPressed()
{
    int64_t value = static_cast<int64_t>(m_LineEdit.text().toLongLong());
    emit PassNewValue(value);
}
