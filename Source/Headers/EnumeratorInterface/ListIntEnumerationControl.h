/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ListIntEnumerationControl.h

  Description: This class inherits by IControlEnumerationHolder. It contains
               all necessary data for int list controls which are enumerated.

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

#ifndef LISTINTENUMERATIONCONTROL_H
#define LISTINTENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class ListIntEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    ListIntEnumerationControl(int32_t id, int32_t value, QList<int64_t> list, QString name, QString unit, bool bIsReadOnly, QWidget *parent);
    // This function updates value in current widget when it's changed somewhere else
    //
    // Parameters:
    // [in] (QList<int64_t>) list - new list values for the widget
    // [in] (int32_t) value - new value for the widget
    void UpdateValue(QList<int64_t> list, int32_t value);

private slots:
    // This slot function is called when the item on the ListWidget was changed
    //
    // Parameters:
    // [in] (const QString &) currentText - string value which is passed to the function from the list widget
    void OnListItemChanged(const QString &currentText);

signals:
    // This signal passes new string value to the Camera class
    //
    // Parameters:
    // [in] (int32_t) id - id of the current control
    // [in] (int64_t) value - new int64 value to be passed
    void PassNewValue(int32_t id, int64_t value);

private:
    // Main element of the class
    QListWidget m_ListWidget;

};

#endif // LISTINTENUMERATIONCONTROL_H
