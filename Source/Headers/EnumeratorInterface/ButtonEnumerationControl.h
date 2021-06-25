#ifndef BUTTONENUMERATIONCONTROL_H
#define BUTTONENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class ButtonEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    ButtonEnumerationControl(int32_t id, QString name, bool bIsReadOnly, QWidget *parent);

signals:
    void PassActionPerform(int32_t id);

private slots:
    void OnButtonClicked();

private:
    QPushButton m_PushButton;

};

#endif // BUTTONENUMERATIONCONTROL_H
