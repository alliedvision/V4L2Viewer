#include "ControlEditWidget.h"

ControlEditWidget::ControlEditWidget(QWidget *parent) : QWidget(parent)
{
    m_pLayout = new QGridLayout;
    setLayout(m_pLayout);
    setStyleSheet("QWidget{"
                  "background-color: qlineargradient(x1:0, y1:0, x1:1, y1:6, stop: 0.9#39393b, stop:1#39393b);"
                  "border:1px solid rgb(41,41,41);"
                  "color:white;}");
}
