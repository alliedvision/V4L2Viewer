#include "ControlsHolderWidget.h"

ControlsHolderWidget::ControlsHolderWidget(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
}

void ControlsHolderWidget::AddElement(IControlEnumerationHolder *controlWidget)
{
    QListWidgetItem *item = new QListWidgetItem(ui.m_ControlsList);
    item->setSizeHint(controlWidget->sizeHint());
    ui.m_ControlsList->setItemWidget(item, controlWidget);
    itemVector.append(controlWidget);
}

void ControlsHolderWidget::RemoveElements()
{
    for (QVector<IControlEnumerationHolder*>::iterator it = itemVector.begin(); it<itemVector.end(); ++it)
    {
        delete *it;
        *it = nullptr;
    }
    itemVector.clear();
}

bool ControlsHolderWidget::IsControlAlreadySet(int32_t id)
{
    for (QVector<IControlEnumerationHolder*>::iterator it = itemVector.begin(); it<itemVector.end(); ++it)
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
    for (QVector<IControlEnumerationHolder*>::iterator it = itemVector.begin(); it<itemVector.end(); ++it)
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

