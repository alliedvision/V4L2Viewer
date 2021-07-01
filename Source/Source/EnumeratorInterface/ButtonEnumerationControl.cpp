#include "ButtonEnumerationControl.h"

ButtonEnumerationControl::ButtonEnumerationControl(int32_t id, QString name, QString unit, bool bIsReadOnly, QWidget *parent) :
    IControlEnumerationHolder(id, name, parent)
{
    m_ControlInfo = QString(tr("%1 control performs action after clicking button below. \n Unit: %2")
                          .arg(name)
                          .arg(unit));
    m_PushButton.setText("Perform");
    m_ControlEditWidget.m_pLayout->addWidget(&m_PushButton);
    if (bIsReadOnly)
    {
        setEnabled(false);
    }
    else
    {
        setEnabled(true);
        connect(&m_PushButton, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
    }
}

void ButtonEnumerationControl::OnButtonClicked()
{
    emit PassActionPerform(m_id);
}
