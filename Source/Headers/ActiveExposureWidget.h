#ifndef ACTIVEEXPOSUREWIDGET_H
#define ACTIVEEXPOSUREWIDGET_H

#include <QWidget>
#include "ui_ActiveExposureWidget.h"

class ActiveExposureWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ActiveExposureWidget(QWidget *parent = nullptr);
    void SetInvert(bool state);
    void SetActive(bool state);
    void SetLineSelectorRange(int32_t currentValue, int32_t range);

    Ui::ActiveExposureWidget ui;
};

#endif // ACTIVEEXPOSUREWIDGET_H
