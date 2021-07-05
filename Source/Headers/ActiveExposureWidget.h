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
    void BlockInvertAndLineSelector(bool state);
    Ui::ActiveExposureWidget ui;

signals:
    void SendInvertState(bool state);
    void SendActiveState(bool state);
    void SendLineSelectorValue(int32_t value);

private slots:
    void OnInvertClicked();
    void OnActiveClicked();
    void OnLineSelectorListItemChanged(const QString &currentText);

};

#endif // ACTIVEEXPOSUREWIDGET_H
