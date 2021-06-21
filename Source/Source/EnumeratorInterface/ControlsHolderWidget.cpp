#include "ControlsHolderWidget.h"

ControlsHolderWidget::ControlsHolderWidget(QWidget *parent) : QWidget(parent)
{
    m_pScrollAreaWidget = new QWidget;
    m_pMainLayout = new QGridLayout;
    m_pScrollArea = new QScrollArea;
    m_pScrollAreaGrid = new QGridLayout;

    m_pScrollAreaWidget->setLayout(m_pScrollAreaGrid);
    m_pMainLayout->addWidget(m_pScrollArea);
    m_pScrollArea->setWidget(m_pScrollAreaWidget);
    m_pScrollAreaGrid->setSizeConstraint(QLayout::SetMinimumSize);
    m_pMainLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
    setLayout(m_pMainLayout);
    setStyleSheet("QWidget{"
                  "background-color: qlineargradient(x1:0, y1:0, x1:1, y1:6, stop: 0.9#39393b, stop:1#39393b);"
                  "border:none;"
                  "color:white;}"
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
                  "background: none;}"
              "QPushButton{"
                  "color:rgb(21,21,21);"
                  "border: 1px solid rgb(94, 94, 94);"
                  "background-color:rgb(125,129,133);"
                  "min-height:30px;"
                  "border-radius:6px;}"
              "QPushButton:hover{"
                  "background-color:rgb(164, 168, 172);}"
                  "QPushButton:pressed{"
                  "background-color:rgb(166, 166, 166);}"
              "QLineEdit{"
                  "border-radius:6px;"
                  "border:1px solid rgb(89,89,89);"
                  "background-color:rgb(37,38,39);"
                  "color:white;"
                  "padding: 1px 0px 1px 3px;}"
              "QLabel{"
                  "background:none;}"
              "QListWidget{"
                  "background-color:rgb(45,45,45);"
                  "border:1px solid rgb(25,25,25);"
                  "border-radius:4px;"
                  "}");
}

void ControlsHolderWidget::AddElement(IControlEnumerationHolder *controlWidget)
{
    m_pScrollAreaGrid->addWidget(controlWidget);
    itemVector.append(controlWidget);
}

void ControlsHolderWidget::RemoveElements()
{
    for(QVector<IControlEnumerationHolder*>::iterator it = itemVector.begin(); it<itemVector.end(); ++it)
    {
        m_pScrollAreaGrid->removeWidget(*it);
        delete *it;
        *it = nullptr;
    }
}

