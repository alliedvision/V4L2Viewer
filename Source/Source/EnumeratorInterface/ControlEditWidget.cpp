#include "ControlEditWidget.h"

ControlEditWidget::ControlEditWidget(QWidget *parent) : QWidget(parent)
{
    m_pLayout = new QGridLayout;
    setLayout(m_pLayout);
    setStyleSheet("QWidget{"
                      "background-color: qlineargradient(x1:0, y1:0, x1:1, y1:6, stop: 0.9#39393b, stop:1#39393b);"
                      "border:1px solid rgb(41,41,41);"
                      "color:white;}"
                  "QSlider{"
                      "background:transparent;"
                      "border:none;}"
                  "QSlider::groove:horizontal{"
                      "border: none;"
                      "background: rgb(41,41,41);"
                      "height:1px;"
                      "border-bottom:1px solid rgb(51,51,51);}"
                  "QSlider::handle:horizontal {"
                      "background: rgb(114,114,114);"
                      "width: 12px;"
                      "height: 18px;"
                      "border-radius: 2px;"
                      "margin: -5px 0;}"
                  "QSlider::sub-page:horizontal {"
                      "background: rgb(0, 100, 207);}"
                  "QSlider::sub-page:horizontal::disabled{"
                      "background: rgb(107, 155, 209);}"
                  "QSlider::groove:horizontal::disabled{"
                      "border: none;"
                      "background: rgb(81,81,81);"
                      "height:1px;"
                      "border-bottom:1px solid rgb(71,71,71);}");
}
