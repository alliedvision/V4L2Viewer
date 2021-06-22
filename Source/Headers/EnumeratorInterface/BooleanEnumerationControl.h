#ifndef BOOLEANENUMERATIONCONTROL_H
#define BOOLEANENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class BooleanEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    BooleanEnumerationControl(int32_t id, bool value, QString name, QWidget *parent);
    void UpdateValue(bool val);

signals:
    void PassNewValue(int32_t id, bool value);

private slots:
    void OnStateChanged(int state);

private:
    QGridLayout m_Layout;
    QLabel m_ControlInfo;
    QCheckBox m_CheckBox;

};

#endif // BOOLEANENUMERATIONCONTROL_H
