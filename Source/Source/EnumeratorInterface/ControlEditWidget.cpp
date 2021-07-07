/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ControlEditWidget.cpp

  Description: This class is a widget, which allows to change the value for all
               of the enumeration controls.

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
