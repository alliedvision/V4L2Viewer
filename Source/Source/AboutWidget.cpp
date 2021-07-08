/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        AboutWidget.cpp

  Description: This widget contains all information about Qt. It can be
               dispalyed in the top menu bar.

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

#include "AboutWidget.h"
#include <QGridLayout>
#include <QPixmap>
#include <QScrollArea>

AboutWidget::AboutWidget(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setStyleSheet("QWidget{"
                "background: transparent;"
                "border:none;"
                "color:rgb(190,190,190);}"
                "QAbstractScrollArea::corner {"
                    "background: transparent;}"
                "QScrollBar:vertical {"
                     "background: transparent;"
                     "width: 13px;"
                     "margin: 0px 0 0px 0;}"
                 "QScrollBar::handle:vertical {"
                     "background: gray;"
                     "min-height: 13px;"
                     "border-radius:6px;}"
                 "QScrollBar::add-line:vertical {"
                     "border: none;"
                     "background: none;"
                     "height: 0;"
                     "subcontrol-position: top;"
                     "subcontrol-origin: margin;}"
                 "QScrollBar::sub-line:vertical {"
                     "border: none;"
                     "background: none;"
                     "height: 0;"
                     "subcontrol-position: top;"
                     "subcontrol-origin: margin;}"
                 "QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {"
                     "border: none;"
                     "width: 0;"
                     "height: 0;"
                     "background: none;}"
                 "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
                     "background: none;}"
                 "QScrollBar:horizontal {"
                     "background: transparent;"
                     "height: 13px;"
                     "margin: 0px 0 0px 0;}"
                 "QScrollBar::handle:horizontal {"
                     "background: gray;"
                     "min-width: 13px;"
                     "border-radius:6px;}"
                 "QScrollBar::add-line:horizontal {"
                     "border: none;"
                     "background: none;"
                     "height: 0;"
                     "subcontrol-position: left;"
                     "subcontrol-origin: margin;}"
                 "QScrollBar::sub-line:horizontal {"
                     "border: none;"
                     "background: none;"
                     "width: 0;"
                     "subcontrol-position: left;"
                     "subcontrol-origin: margin;}"
                 "QScrollBar::up-arrow:horizontal, QScrollBar::down-arrow:horizontal {"
                     "border: none;"
                     "width: 0;"
                     "height: 0;"
                     "background: none;}"
                 "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
                     "background: none;}");


    QPixmap pixmap;
    pixmap.load(":/V4L2Viewer/Qt_logo_2016.png");
    pixmap = pixmap.scaled(150, 150, Qt::KeepAspectRatio);

    QLabel *labelImg = new QLabel(this);
    labelImg->setTextFormat(Qt::RichText);
    labelImg->setPixmap(pixmap);

    m_TextLabel = new QLabel(this);
    m_TextLabel->setOpenExternalLinks(true);
    m_TextLabel->setWordWrap(true);
    m_TextLabel->setTextFormat(Qt::RichText);
    m_TextLabel->setText(tr("This program uses Qt version 5.13.2 <br><br>"
                   "Qt is a C++ toolkit for cross-platform application development. <br><br>"
                   "Qt provides single-source portability across all major desktop operating systems. "
                   "It is also available for embedded Linux and other embedded and mobile operating systems. <br><br>"
                   "Qt is available under multiple licensing options designed to accommodate the needs of our various users. <br><br>"
                   "Qt licensed under our commercial license agreement is appropriate for development of proprietary/commercial "
                   "software where you do not want to share any source code with third parties or otherwise cannot comply "
                   "with the terms of GNU (L)GPL.<br><br>"
                   "Qt licensed under GNU (L)GPL is appropriate for the development of Qt applications provided you can comply "
                   "with the terms and conditions of the respective licenses.<br><br>"
                   "Please see <a>qt.io/icensing</a> for and overview of Qt licensing.<br><br>"
                   "Copyright (C) 2020 The Qt Company Ltd and other contributors.<br><br>"
                   "Qt and the Qt logo are trademarks of The Qt Company Ltd.<br><br>"
                   "Qt is The Qt Company Ltd product developed as an open source project. See <a>qt.io</a> for more information."));

    QGridLayout *scrollAreaLayout = new QGridLayout(this);
    scrollAreaLayout->addWidget(labelImg, 0, 0);
    scrollAreaLayout->addWidget(m_TextLabel, 0, 1);

    QWidget *widget = new QWidget(this);
    widget->setLayout(scrollAreaLayout);

    QScrollArea *scrollArea = new QScrollArea(this);
    //scrollArea->setLayout(scrollAreaLayout);
    scrollArea->setWidget(widget);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(scrollArea);

    this->setLayout(layout);
}

void AboutWidget::UpdateStrings()
{
    m_TextLabel->setText(tr("This program uses Qt version 5.13.2 <br><br>"
                   "Qt is a C++ toolkit for cross-platform application development. <br><br>"
                   "Qt provides single-source portability across all major desktop operating systems. "
                   "It is also available for embedded Linux and other embedded and mobile operating systems. <br><br>"
                   "Qt is available under multiple licensing options designed to accommodate the needs of our various users. <br><br>"
                   "Qt licensed under our commercial license agreement is appropriate for development of proprietary/commercial "
                   "software where you do not want to share any source code with third parties or otherwise cannot comply "
                   "with the terms of GNU (L)GPL.<br><br>"
                   "Qt licensed under GNU (L)GPL is appropriate for the development of Qt applications provided you can comply "
                   "with the terms and conditions of the respective licenses.<br><br>"
                   "Please see <a>qt.io/icensing</a> for and overview of Qt licensing.<br><br>"
                   "Copyright (C) 2020 The Qt Company Ltd and other contributors.<br><br>"
                   "Qt and the Qt logo are trademarks of The Qt Company Ltd.<br><br>"
                            "Qt is The Qt Company Ltd product developed as an open source project. See <a>qt.io</a> for more information."));
}


