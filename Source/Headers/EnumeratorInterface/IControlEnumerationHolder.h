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

protected:
    QLabel  m_NameOfControl;
    QWidget m_MainWidget;
    QGridLayout m_Layout;
    int32_t m_id;
};

#endif // ICONTROLENUMERATIONHOLDER_H
