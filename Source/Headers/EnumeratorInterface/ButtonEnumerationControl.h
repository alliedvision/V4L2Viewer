/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ButtonEnumerationControl.h

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

#ifndef BUTTONENUMERATIONCONTROL_H
#define BUTTONENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class ButtonEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    ButtonEnumerationControl(int32_t id, QString name, QString unit, bool bIsReadOnly, QWidget *parent);

signals:
    // This signal passes info about action which needs to be performed
    //
    // Parameters:
    // [in] (int32_t) id - id of the control
    void PassActionPerform(int32_t id);

private slots:
    // This slot function is called after button is pressed
    void OnButtonClicked();

private:
    // Main element of the class, after pressing the action is going to be performed.
    QPushButton m_PushButton;
};

#endif // BUTTONENUMERATIONCONTROL_H
