#include "SettingsActionWidget.h"

SettingsActionWidget::SettingsActionWidget(QWidget *parent) : QWidget(parent)
{
    m_pRedBalance = new CustomOptionWidget("RedBalance:", this);
    m_pBlueBalance = new CustomOptionWidget("BlueBalance", this);
    m_pSaturation = new CustomOptionWidget("Saturation", this);
    m_pHue = new CustomOptionWidget("Hue:", this);
    m_pContrast = new CustomOptionWidget("Contrast:", this);
    m_pMainLyout = new QGridLayout(this);
    m_pMainLyout->setMargin(6);
    setStyleSheet("QLabel{"
                  "background:none;}"
                  "QLabel::disabled{"
                  "color: rgb(45, 45, 45);}"
                  "QLineEdit{"
                  "border-radius:6px;"
                  "border:1px solid rgb(89,89,89);"
                  "background-color:rgb(37,38,39);"
                  "color:white;"
                  "padding: 1px 0px 1px 3px;}"
                  "QLineEdit::disabled{"
                  "background-color:rgb(67,68,69);"
                  "color:rgb(87,88,89);}");
    m_pMainLyout->addWidget(m_pRedBalance,  0, 0);
    m_pMainLyout->addWidget(m_pBlueBalance, 1, 0);
    m_pMainLyout->addWidget(m_pSaturation,  2, 0);
    m_pMainLyout->addWidget(m_pHue,         3, 0);
    m_pMainLyout->addWidget(m_pContrast,    4, 0);
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

CustomOptionWidget *SettingsActionWidget::GetContrastWidget()
{
    return m_pContrast;
}
