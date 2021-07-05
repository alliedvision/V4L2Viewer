#ifndef SETTINGSACTIONWIDGET_H
#define SETTINGSACTIONWIDGET_H

#include <QWidget>
#include "CustomOptionWidget.h"

class SettingsActionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsActionWidget(QWidget *parent = nullptr);
    CustomOptionWidget *GetRedBalanceWidget();
    CustomOptionWidget *GetBlueBalanceWidget();
    CustomOptionWidget *GetSaturationWidget();
    CustomOptionWidget *GetHueWidget();
    CustomOptionWidget *GetContrastWidget();

private:
    CustomOptionWidget *m_pRedBalance;
    CustomOptionWidget *m_pBlueBalance;
    CustomOptionWidget *m_pSaturation;
    CustomOptionWidget *m_pHue;
    CustomOptionWidget *m_pContrast;
    QGridLayout *m_pMainLyout;
};

#endif // SETTINGSACTIONWIDGET_H