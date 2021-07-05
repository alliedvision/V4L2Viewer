#ifndef INTEGER64ENUMERATIONCONTROL_H
#define INTEGER64ENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class Integer64EnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    Integer64EnumerationControl(int32_t id, int64_t min, int64_t max, int64_t value, QString name, QString unit, bool bIsReadOnly, QWidget *parent);
    void UpdateValue(int64_t value);

signals:
    void PassNewValue(int32_t id, int64_t value);
    void PassSliderValue(int32_t id, int64_t value);

private slots:
    void OnLineEditPressed();
    void OnSliderValueChanged(int value);
    void OnSliderLogValueChanged(int value);
    void OnSliderReleased();

private:
    int32_t GetSliderLogValue(int64_t value);

    QSlider m_Slider;
    QLineEdit m_LineEdit;

    int64_t m_Min;
    int64_t m_Max;
    int64_t m_Value;
    int64_t m_SliderValue;
};

#endif // INTEGER64ENUMERATIONCONTROL_H