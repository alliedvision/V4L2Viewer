#ifndef ICONTROLENUMERATIONHOLDER_H
#define ICONTROLENUMERATIONHOLDER_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QCheckBox>
#include <QPushButton>

class IControlEnumerationHolder : public QWidget
{
    Q_OBJECT
public:
    explicit IControlEnumerationHolder(int32_t id, QString name, QWidget *parent = nullptr);
    virtual ~IControlEnumerationHolder() = 0;
    int32_t GetWidgetControlId();

protected:

    QLabel  m_NameOfControl;
    QLabel  m_ControlInfo;
    QWidget m_ControlWidget;
    QGridLayout m_MainLayout;
    QGridLayout m_ControlWidgetLayout;
    int32_t m_id;
};

#endif // ICONTROLENUMERATIONHOLDER_H
