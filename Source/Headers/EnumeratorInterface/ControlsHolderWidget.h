/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ControlsHolderWidget.h

  Description: This class is a widget, which contains all of the enumeration
               controls displayed in a list.

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

#ifndef CONTROLSHOLDERWIDGET_H
#define CONTROLSHOLDERWIDGET_H

#include <QDialog>
#include <QScrollArea>
#include <QGridLayout>
#include <EnumeratorInterface/IControlEnumerationHolder.h>
#include "ui_ControlsHolderWidget.h"

class ControlsHolderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControlsHolderWidget(QWidget *parent = nullptr);
    // This function add new enumeration control to the list
    //
    // Parameters:
    // [in] (IControlEnumerationHolder *) controlWidget - element which is dynamically casted to the specific control
    void AddElement(IControlEnumerationHolder *controlWidget);
    // This function removes all of the elements from the list, for example when camera is closed
    void RemoveElements();
    // This function returns bool state which depends on whether control is already set or not. This is useful,
    // when updating some of the values
    //
    // Parameters:
    // [in] (int32_t) id - control id to be checked
    //
    // Returns:
    // (bool) - boolean value
    bool IsControlAlreadySet(int32_t id);
    // This function returns IControlEnumerationHolder* depending on the passed id value
    //
    // Parameters:
    // [in] (int32_t) id - control id value
    // [in] (bool &) bIsSuccess - boolean value which ensures about found item
    //
    // Returns:
    // (IControlEnumerationHolder*) - found object
    IControlEnumerationHolder* GetControlWidget(int32_t id, bool &bIsSuccess);

    Ui::ControlsHolderWidget ui;

private slots:
    // This slot function is called when the element on the list changed
    //
    // Parameters:
    // [in] row - current row
    void OnListItemChanged(int row);

private:
    // This vector stores all of the controls widgets
    QVector<IControlEnumerationHolder*> m_itemVector;
};


#endif // CONTROLSHOLDERWIDGET_H
