#include "BooleanEnumerationControl.h"

BooleanEnumerationControl::BooleanEnumerationControl(int32_t id, bool value, QString name, QString unit, bool bIsReadOnly, QWidget *parent):
    IControlEnumerationHolder(id, name, parent)
{
    m_ControlInfo = QString(tr("%1 control accepts boolean values. \n Unit: %2")
                          .arg(name)
                          .arg(unit));
    m_CheckBox.setText("On/Off");
    m_CheckBox.setChecked(value);
    m_ControlEditWidget.m_pLayout->addWidget(&m_CheckBox);
    m_CurrentValue.setText(value ? "True" : "False");
    if (bIsReadOnly)
    {
        setEnabled(false);
        m_ControlInfo += tr("\n Control is READONLY");
    }
    else
    {
        setEnabled(true);
        connect(&m_CheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnStateChanged(int)));
    }
}

void BooleanEnumerationControl::UpdateValue(bool val)
{
    m_CheckBox.blockSignals(true);
    m_CheckBox.setChecked(val);
    m_CurrentValue.setText(val ? "True" : "False");
    m_CheckBox.blockSignals(false);
}

void BooleanEnumerationControl::OnStateChanged(int state)
{
    if (state == Qt::Unchecked)
    {
        emit PassNewValue(m_id, false);
    }
    else
    {
        emit PassNewValue(m_id, true);
    }
}
