#ifndef INTEGERENUMERATIONCONTROL_H
#define INTEGERENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class IntegerEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    IntegerEnumerationControl(int32_t id, int32_t min, int32_t max, int32_t value, QString name, bool bIsReadOnly, QWidget *parent);
    void UpdateValue(int32_t value);

signals:
    void PassNewValue(int32_t id, int32_t value);

private slots:
    void OnLineEditPressed();

private:
    QLineEdit m_LineEdit;

    int32_t m_Min;
    int32_t m_Max;
    int32_t m_Value;
};


#endif // INTEGERENUMERATIONCONTROL_H
