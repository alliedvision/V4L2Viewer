/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ButtonEnumerationControl.cpp

  Description: This class inherits by IControlEnumerationHolder. It contains
               all necessary data for button controls which are enumerated.

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

#include "ButtonEnumerationControl.h"

ButtonEnumerationControl::ButtonEnumerationControl(int32_t id, QString name, QString unit, bool bIsReadOnly, QWidget *parent) :
    IControlEnumerationHolder(id, name, parent)
{
    m_ControlInfo = QString(tr("%1 control performs action after clicking button below. \n Unit: %2")
                          .arg(name)
                          .arg(unit));
    m_PushButton.setText("Perform");

    m_MainLayout.addWidget(&m_NameOfControl, 0, 0);
    m_MainLayout.addWidget(&m_PushButton, 0, 1);

    m_MainLayout.setColumnStretch(0, 1);
    m_MainLayout.setColumnStretch(1, 0);

    if (bIsReadOnly)
    {
        setEnabled(false);
    }
    else
    {
        setEnabled(true);
        connect(&m_PushButton, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
    }
}

void ButtonEnumerationControl::OnButtonClicked()
{
    emit PassActionPerform(m_id);
}
