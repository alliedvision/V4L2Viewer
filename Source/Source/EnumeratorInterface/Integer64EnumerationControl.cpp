#include "Integer64EnumerationControl.h"

Integer64EnumerationControl::Integer64EnumerationControl(int32_t id, int64_t min, int64_t max, int64_t value, QString name, bool bIsReadOnly, QWidget *parent):
    IControlEnumerationHolder(id, name, parent),
    m_Min(min),
    m_Max(max),
    m_Value(value)
{
    m_ControlInfo.setText(QString(tr("%1 control accepts 64-bit integers. \n Minimum: %2 \n Maximum: %3")
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

void Integer64EnumerationControl::UpdateValue(int64_t value)
{
    m_LineEdit.blockSignals(true);
    m_LineEdit.setText(QString::number(value));
    m_LineEdit.blockSignals(false);
}

void Integer64EnumerationControl::OnLineEditPressed()
{
    int64_t value = static_cast<int64_t>(m_LineEdit.text().toLongLong());
    emit PassNewValue(m_id, value);
}
