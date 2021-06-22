#include "IntegerEnumerationControl.h"

IntegerEnumerationControl::IntegerEnumerationControl(int32_t id, int32_t min, int32_t max, int32_t value, QString name, bool bIsReadOnly, QWidget *parent):
    IControlEnumerationHolder(id, name, parent),
    m_Min(min),
    m_Max(max),
    m_Value(value)
{
    m_ControlInfo.setText(QString(tr("%1 control accepts 32-bit integers. \n Minimum: %2 \n Maximum: %3")
                          .arg(name)
                          .arg(m_Min)
                          .arg(m_Max)));
    m_ControlWidgetLayout.addWidget(&m_LineEdit);
    m_LineEdit.setValidator(new QIntValidator(m_Min, m_Max, this));
    m_LineEdit.setText(QString::number(m_Value));
    if (bIsReadOnly)
    {
        setEnabled(false);
    }
    else
    {
        setEnabled(true);
        connect(&m_LineEdit, SIGNAL(returnPressed()), this, SLOT(OnLineEditPressed()));
    }
}

void IntegerEnumerationControl::UpdateValue(int32_t value)
{
    m_LineEdit.blockSignals(true);
    m_LineEdit.setText(QString::number(value));
    m_LineEdit.blockSignals(false);
}

void IntegerEnumerationControl::OnLineEditPressed()
{
    int32_t value = m_LineEdit.text().toInt();
    emit PassNewValue(m_id, value);
}
