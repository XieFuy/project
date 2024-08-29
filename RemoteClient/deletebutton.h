#ifndef DELETEBUTTON_H
#define DELETEBUTTON_H

#include <QDialog>

namespace Ui {
class DeleteButton;
}

class DeleteButton : public QDialog //删除按钮类，进行鼠标右击的时候生成的删除按钮
{
    Q_OBJECT

public:
    explicit DeleteButton(QWidget *parent = 0);
    ~DeleteButton();
signals:
    void buttonClick();
private:
    Ui::DeleteButton *ui;
};

#endif // DELETEBUTTON_H
