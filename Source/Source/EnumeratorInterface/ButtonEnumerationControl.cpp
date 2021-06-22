#include "ButtonEnumerationControl.h"

ButtonEnumerationControl::ButtonEnumerationControl(int32_t id, QString name, QWidget *parent) :
    IControlEnumerationHolder(id, name, parent)
{
    m_ControlInfo.setText(QString(tr("%1 control performs action after clicking button below.")
                          .arg(name)));
    m_PushButton.setText("Perform");
    m_ControlWidgetLayout.addWidget(&m_PushButton);
    connect(&m_PushButton, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
}

void ButtonEnumerationControl::OnButtonClicked()
{
    emit PassActionPerform(m_id);
}
