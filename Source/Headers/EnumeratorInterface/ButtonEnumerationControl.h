#ifndef BUTTONENUMERATIONCONTROL_H
#define BUTTONENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class ButtonEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    ButtonEnumerationControl(QString name, QWidget *parent);

signals:
    void PassActionPerform();

private slots:
    void OnButtonClicked();

private:
    QGridLayout m_Layout;
    QLabel m_ControlInfo;
    QPushButton m_PushButton;

};

#endif // BUTTONENUMERATIONCONTROL_H
