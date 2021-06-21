#include "ButtonEnumerationControl.h"

ButtonEnumerationControl::ButtonEnumerationControl(int32_t id, QString name, QWidget *parent) :
    IControlEnumerationHolder(id, name, parent)
{
    m_ControlInfo.setText("Control info here");
    m_PushButton.setText("Perform");
    m_Layout.addWidget(&m_ControlInfo, 0, 0);
    m_Layout.addWidget(&m_PushButton, 1, 0);
    m_MainWidget.setLayout(&m_Layout);
    connect(&m_PushButton, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
}

void ButtonEnumerationControl::OnButtonClicked()
{
    emit PassActionPerform();
}
