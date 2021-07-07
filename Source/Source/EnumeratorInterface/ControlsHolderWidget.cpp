#include "ControlsHolderWidget.h"
#include <QDebug>

ControlsHolderWidget::ControlsHolderWidget(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
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

void ControlsHolderWidget::closeEvent(QCloseEvent *event)
{
    for (QVector<IControlEnumerationHolder*>::iterator it = m_itemVector.begin(); it<m_itemVector.end(); ++it)
    {
        (*it)->CloseControlEditWidget();
    }
    ControlsHolderWidget::close();
}

void ControlsHolderWidget::OnListItemChanged(int row)
{
    QString info = dynamic_cast<IControlEnumerationHolder *>(ui.m_ControlsList->itemWidget(ui.m_ControlsList->item(row)))->GetControlInfo();
    ui.m_DescriptionLabel->setText(info);
}

