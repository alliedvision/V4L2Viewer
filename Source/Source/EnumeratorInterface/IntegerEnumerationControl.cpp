#include "IntegerEnumerationControl.h"

IntegerEnumerationControl::IntegerEnumerationControl(int32_t id, int32_t min, int32_t max, int32_t step, int32_t value, QString name, QWidget *parent):
    IControlEnumerationHolder(id, name, parent),
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

void IntegerEnumerationControl::OnLineEditPressed()
{
    int32_t value = m_LineEdit.text().toInt();
    emit PassNewValue(value);
}
