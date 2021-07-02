#include "ActiveExposureWidget.h"

ActiveExposureWidget::ActiveExposureWidget(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
}

void ActiveExposureWidget::SetInvert(bool state)
{
    ui.m_InvertCheckBox->blockSignals(true);
    ui.m_InvertCheckBox->setChecked(state);
    ui.m_InvertCheckBox->blockSignals(false);
}

void ActiveExposureWidget::SetActive(bool state)
{
    ui.m_ActiveCheckBox->blockSignals(true);
    ui.m_ActiveCheckBox->setChecked(state);
    ui.m_ActiveCheckBox->blockSignals(false);
}

void ActiveExposureWidget::SetLineSelectorRange(int32_t currentValue, int32_t range)
{
    ui.m_ListWidget->blockSignals(true);
    for (int i=0; i<range; ++i)
    {
        ui.m_ListWidget->addItem(QString::number(i));
    }
    ui.m_ListWidget->setCurrentRow(currentValue);
    ui.m_ListWidget->blockSignals(false);
}
