/* Allied Vision V4L2Viewer - Graphical Video4Linux Viewer Example
   Copyright (C) 2021 Allied Vision Technologies GmbH

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.  */


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
