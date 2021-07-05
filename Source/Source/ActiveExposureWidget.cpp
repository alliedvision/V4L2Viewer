#include "ActiveExposureWidget.h"

ActiveExposureWidget::ActiveExposureWidget(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);

    connect(ui.m_InvertCheckBox, SIGNAL(clicked()), this, SLOT(OnInvertClicked()));
    connect(ui.m_ActiveCheckBox, SIGNAL(clicked()), this, SLOT(OnActiveClicked()));
    connect(ui.m_ListWidget,    SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnLineSelectorListItemChanged(const QString &)));
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

void ActiveExposureWidget::BlockInvertAndLineSelector(bool state)
{
    ui.m_InvertCheckBox->setEnabled(!state);
    ui.m_ListWidget->setEnabled(!state);
}

void ActiveExposureWidget::OnInvertClicked()
{
    bool state = ui.m_InvertCheckBox->isChecked();
    ui.m_InvertCheckBox->setChecked(state);
    emit SendInvertState(state);
}

void ActiveExposureWidget::OnActiveClicked()
{
    bool state = ui.m_ActiveCheckBox->isChecked();
    ui.m_ActiveCheckBox->setChecked(state);
    emit SendActiveState(state);
}

void ActiveExposureWidget::OnLineSelectorListItemChanged(const QString &currentText)
{
    int32_t value = static_cast<int32_t>(currentText.toInt());
    emit SendLineSelectorValue(value);
}
