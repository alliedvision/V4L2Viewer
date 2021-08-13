/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ControlsHolderWidget.cpp

  Description: This class is a widget, which contains all of the enumeration
               controls displayed in a list.

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

#include "ControlsHolderWidget.h"

ControlsHolderWidget::ControlsHolderWidget(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    connect(ui.m_ControlsList, SIGNAL(currentRowChanged(int)), this, SLOT(OnListItemChanged(int)));
}

void ControlsHolderWidget::AddElement(IControlEnumerationHolder *controlWidget)
{
    ui.m_ControlsList->blockSignals(true);
    QListWidgetItem *item = new QListWidgetItem(ui.m_ControlsList);
    item->setSizeHint(controlWidget->sizeHint());
    ui.m_ControlsList->setItemWidget(item, controlWidget);
    m_itemVector.append(controlWidget);
    ui.m_ControlsList->blockSignals(false);
}

void ControlsHolderWidget::RemoveElements()
{
    ui.m_ControlsList->blockSignals(true);
    for (QVector<IControlEnumerationHolder*>::iterator it = m_itemVector.begin(); it<m_itemVector.end(); ++it)
    {
        delete *it;
        *it = nullptr;
    }
    ui.m_ControlsList->clear();
    m_itemVector.clear();
    ui.m_ControlsList->blockSignals(false);
}

bool ControlsHolderWidget::IsControlAlreadySet(int32_t id)
{
    for (QVector<IControlEnumerationHolder*>::iterator it = m_itemVector.begin(); it<m_itemVector.end(); ++it)
    {
        if((*it)->GetWidgetControlId() == id)
        {
            return true;
        }
    }
    return false;
}

IControlEnumerationHolder* ControlsHolderWidget::GetControlWidget(int32_t id, bool &bIsSuccess)
{
    for (QVector<IControlEnumerationHolder*>::iterator it = m_itemVector.begin(); it<m_itemVector.end(); ++it)
    {
        if((*it)->GetWidgetControlId() == id)
        {
            bIsSuccess = true;
            return (*it);
        }
    }
    bIsSuccess = false;
    return nullptr;
}

QSize ControlsHolderWidget::sizeHint() const
{
    QSize size;
    size.setWidth(500);
    size.setHeight(this->height());
    return size;
}

void ControlsHolderWidget::OnListItemChanged(int row)
{
    QString info = dynamic_cast<IControlEnumerationHolder *>(ui.m_ControlsList->itemWidget(ui.m_ControlsList->item(row)))->GetControlInfo();
    ui.m_DescriptionLabel->setText(info);
}
