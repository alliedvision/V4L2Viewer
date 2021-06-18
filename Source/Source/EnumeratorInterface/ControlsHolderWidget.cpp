#include "ControlsHolderWidget.h"

ControlsHolderWidget::ControlsHolderWidget(QWidget *parent) : QWidget(parent)
{
    m_pScrollAreaWidget = new QWidget;
    m_pMainLayout = new QGridLayout;
    m_pScrollArea = new QScrollArea;
    m_pScrollAreaGrid = new QGridLayout;

    m_pScrollAreaWidget->setLayout(m_pScrollAreaGrid);
    m_pMainLayout->addWidget(m_pScrollArea);
    m_pScrollArea->setWidget(m_pScrollAreaWidget);
    m_pScrollAreaGrid->setSizeConstraint(QLayout::SetMinimumSize);
    m_pMainLayout->setSizeConstraint(QLayout::SetMinimumSize);
    setLayout(m_pMainLayout);
}

void ControlsHolderWidget::AddElement(IControlEnumerationHolder *controlWidget)
{
    m_pScrollAreaGrid->addWidget(controlWidget);
    itemVector.append(controlWidget);
}

void ControlsHolderWidget::RemoveElements()
{
    for(QVector<IControlEnumerationHolder*>::iterator it = itemVector.begin(); it<itemVector.end(); ++it)
    {
        m_pScrollAreaGrid->removeWidget(*it);
        delete *it;
        *it = nullptr;
    }
}

