#ifndef INTEGER64ENUMERATIONCONTROL_H
#define INTEGER64ENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class Integer64EnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    Integer64EnumerationControl(int32_t id, int64_t min, int64_t max, int64_t step, int64_t value, QString name, QWidget *parent);
    void UpdateValue(int64_t value);

signals:
    void PassNewValue(int32_t id, int64_t value);

private slots:
    void OnLineEditPressed();

private:
    QGridLayout m_Layout;
    QLabel m_ControlInfo;
    QLineEdit m_LineEdit;

    int64_t m_Min;
    int64_t m_Max;
    int64_t m_Step;
    int64_t m_Value;
};

#endif // INTEGER64ENUMERATIONCONTROL_H
