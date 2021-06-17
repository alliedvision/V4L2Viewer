#ifndef INTEGERENUMERATIONCONTROL_H
#define INTEGERENUMERATIONCONTROL_H

#include "EnumeratorInterface/IControlEnumerationHolder.h"

class IntegerEnumerationControl : public IControlEnumerationHolder
{
public:
    IntegerEnumerationControl(QString name, QWidget *parent);
    QLineEdit &GetLineEdit();

private:
    QGridLayout m_Layout;
    QLabel m_ControlInfo;
    QLineEdit m_LineEdit;
};

#endif // INTEGERENUMERATIONCONTROL_H
