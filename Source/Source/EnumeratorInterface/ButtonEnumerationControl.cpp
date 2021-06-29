#include "ButtonEnumerationControl.h"

ButtonEnumerationControl::ButtonEnumerationControl(int32_t id, QString name, bool bIsReadOnly, QWidget *parent) :
    IControlEnumerationHolder(id, name, parent)
{
    m_ControlInfo = QString(tr("%1 control performs action after clicking button below.")
                          .arg(name));
    m_PushButton.setText("Perform");
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
