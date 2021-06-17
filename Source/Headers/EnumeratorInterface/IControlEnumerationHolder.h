#ifndef ICONTROLENUMERATIONHOLDER_H
#define ICONTROLENUMERATIONHOLDER_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>

class IControlEnumerationHolder : public QWidget
{
    Q_OBJECT
public:
    explicit IControlEnumerationHolder(QString name, QWidget *parent = nullptr);
    virtual ~IControlEnumerationHolder() = 0;

protected:
    QLabel  m_NameOfControl;
    QWidget m_MainWidget;
    QGridLayout m_Layout;
};

#endif // ICONTROLENUMERATIONHOLDER_H
