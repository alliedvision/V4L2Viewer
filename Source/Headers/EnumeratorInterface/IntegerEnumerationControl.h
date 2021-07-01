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
    void PassSliderValue(int32_t id, int32_t value);

private slots:
    void OnLineEditPressed();
    void OnSliderValueChanged(int value);
    void OnSliderLogValueChanged(int value);
    void OnSliderReleased();

private:
    int32_t GetSliderLogValue(int32_t value);

    QSlider m_Slider;
    QLineEdit m_LineEdit;
    int32_t m_Min;
    int32_t m_Max;
    int32_t m_Value;
    int32_t m_SliderValue;
};


#endif // INTEGERENUMERATIONCONTROL_H
