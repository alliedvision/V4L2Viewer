#include "ControlsHolderWidget.h"

ControlsHolderWidget::ControlsHolderWidget(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
    m_pWidgetLayout = new QGridLayout;
    ui.m_ScrollAreaWidget->setLayout(m_pWidgetLayout);
}

void ControlsHolderWidget::AddElement(IControlEnumerationHolder *controlWidget)
{
    m_pWidgetLayout->addWidget(controlWidget);
    itemVector.append(controlWidget);
}

void ControlsHolderWidget::RemoveElements()
{
    for (QVector<IControlEnumerationHolder*>::iterator it = itemVector.begin(); it<itemVector.end(); ++it)
    {
        m_pWidgetLayout->removeWidget(*it);
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

