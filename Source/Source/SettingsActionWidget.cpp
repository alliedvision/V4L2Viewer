#include "SettingsActionWidget.h"

SettingsActionWidget::SettingsActionWidget(QWidget *parent) : QWidget(parent)
{
    m_pRedBalance = new CustomOptionWidget("RedBalance:", this);
    m_pBlueBalance = new CustomOptionWidget("BlueBalance", this);
    m_pSaturation = new CustomOptionWidget("Saturation", this);
    m_pHue = new CustomOptionWidget("Hue:", this);
    m_pMainLyout = new QGridLayout(this);
    m_pMainLyout->addWidget(m_pRedBalance,  0, 0);
    m_pMainLyout->addWidget(m_pBlueBalance, 1, 0);
    m_pMainLyout->addWidget(m_pSaturation,  2, 0);
    m_pMainLyout->addWidget(m_pHue,         3, 0);
    setLayout(m_pMainLyout);
}

CustomOptionWidget *SettingsActionWidget::GetRedBalanceWidget()
{
    return m_pRedBalance;
}

CustomOptionWidget *SettingsActionWidget::GetBlueBalanceWidget()
{
    return m_pBlueBalance;
}

CustomOptionWidget *SettingsActionWidget::GetSaturationWidget()
{
    return m_pSaturation;
}

CustomOptionWidget *SettingsActionWidget::GetHueWidget()
{
    return m_pHue;
}
