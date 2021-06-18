#ifndef BOOLEANENUMERATIONCONTROL_H
#define BOOLEANENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class BooleanEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    BooleanEnumerationControl(bool value, QString name, QWidget *parent);

signals:
    void PassNewValue(bool value);

private slots:
    void OnStateChanged(int state);

private:
    QGridLayout m_Layout;
    QLabel m_ControlInfo;
    QCheckBox m_CheckBox;

};

#endif // BOOLEANENUMERATIONCONTROL_H
