/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ActiveExposureWidget.cpp

  Description: This widget class stores widgets for updating active exposure
               control.

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

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

void ActiveExposureWidget::SetLineSelectorRange(int32_t currentValue, int32_t min, int32_t max, int32_t step)
{
    ui.m_ListWidget->blockSignals(true);
    ui.m_ListWidget->clear();
    for (int32_t i=min; i<=max; i+=step)
    {
        ui.m_ListWidget->addItem(QString("GPIO ")+QString::number(i));
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
    QStringList list = currentText.split(QLatin1Char(' '));
    if (list.size() > 1)
    {
        bool bIsConversionSuccessful = false;
        int32_t value = static_cast<int32_t>(list.at(1).toInt(&bIsConversionSuccessful));
        if (bIsConversionSuccessful)
        {
            emit SendLineSelectorValue(value);
        }
    }
}

void ActiveExposureWidget::closeEvent(QCloseEvent *event)
{
    emit CloseSignal();
    QWidget::closeEvent(event);
}
