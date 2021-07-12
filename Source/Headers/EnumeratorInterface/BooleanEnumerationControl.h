/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        BooleanEnumerationControl.h

  Description: This class inherits by IControlEnumerationHolder. It contains
               all necessary data for boolean controls which are enumerated.

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

#ifndef BOOLEANENUMERATIONCONTROL_H
#define BOOLEANENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class BooleanEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    BooleanEnumerationControl(int32_t id, bool value, QString name, QString unit, bool bIsReadOnly, QWidget *parent);
    // This function updates value in current widget when it's changed somewhere else
    //
    // Parameters:
    // [in] (bool) val - new value for the control
    void UpdateValue(bool val);

signals:
    // This signal passes new value to the Camera class
    //
    // Parameters:
    // [in] (int32_t) id - id of the control
    // [in] (bool) value - new value to be passed
    void PassNewValue(int32_t id, bool value);

private slots:
    // This slot function is called when the item in the comboBox is changed
    //
    // Parameters:
    // [in] (const QString &) text - text from the comboBox
    void OnTextChanged(const QString &text);

private:
    QComboBox m_ComboBox;
};

#endif // BOOLEANENUMERATIONCONTROL_H
