#ifndef INTEGERENUMERATIONCONTROL_H
#define INTEGERENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class IntegerEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    IntegerEnumerationControl(int32_t id, int32_t min, int32_t max, int32_t step, int32_t value, QString name, QWidget *parent);

signals:
    void PassNewValue(int32_t value);

private slots:
    void OnLineEditPressed();

private:
    QGridLayout m_Layout;
    QLabel m_ControlInfo;
    QLineEdit m_LineEdit;

    int32_t m_Min;
    int32_t m_Max;
    int32_t m_Step;
    int32_t m_Value;
};


#endif // INTEGERENUMERATIONCONTROL_H
