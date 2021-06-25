#include "BooleanEnumerationControl.h"

BooleanEnumerationControl::BooleanEnumerationControl(int32_t id, bool value, QString name, bool bIsReadOnly, QWidget *parent):
    IControlEnumerationHolder(id, name, parent)
{
    m_ControlInfo.setText(QString(tr("%1 control accepts boolean values.")
                          .arg(name)));
    m_CheckBox.setText("On/Off");
    m_CheckBox.setChecked(value);
    m_ControlWidgetLayout.addWidget(&m_CheckBox);
    if (bIsReadOnly)
    {
        setEnabled(false);
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
