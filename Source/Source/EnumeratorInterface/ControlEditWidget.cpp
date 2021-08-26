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
