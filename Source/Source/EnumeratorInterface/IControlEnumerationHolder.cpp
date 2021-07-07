/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        IControlEnumerationHolder.cpp

  Description: This interface class describes how the derived control
               classes should looks like.

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

#include "IControlEnumerationHolder.h"
#include <QMouseEvent>

IControlEnumerationHolder::IControlEnumerationHolder(int32_t id, QString name, QWidget *parent) :
    QWidget(parent),
    m_id(id)
{
    m_NameOfControl.setText(name);
    m_MainLayout.addWidget(&m_NameOfControl, 0, 0);
    m_MainLayout.addWidget(&m_CurrentValue, 0, 1);
    setLayout(&m_MainLayout);
    m_MainLayout.setMargin(0);
}

IControlEnumerationHolder::~IControlEnumerationHolder()
{

}

int32_t IControlEnumerationHolder::GetWidgetControlId()
{
    return m_id;
}

QString IControlEnumerationHolder::GetControlInfo()
{
    return m_ControlInfo;
}

void IControlEnumerationHolder::CloseControlEditWidget()
{
    if (!m_ControlEditWidget.isHidden())
    {
        m_ControlEditWidget.close();
    }
}

void IControlEnumerationHolder::mouseDoubleClickEvent(QMouseEvent *event)
{
    QPoint posGlobal = mapToGlobal(event->pos());
    m_ControlEditWidget.setGeometry(posGlobal.x(), posGlobal.y(), this->width(), this->height());
    m_ControlEditWidget.setWindowTitle(m_NameOfControl.text());
    m_ControlEditWidget.show();
}
