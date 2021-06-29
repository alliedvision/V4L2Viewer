#include "ControlEditWidget.h"

ControlEditWidget::ControlEditWidget(QWidget *parent) : QWidget(parent)
{
    m_pLayout = new QGridLayout;
    setLayout(m_pLayout);
}
