#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include"cclientcontorler.h"
#include<QMessageBox>

namespace Ui {
class CLoginWidget;
}

class CLoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CLoginWidget(QWidget *parent = 0);
    ~CLoginWidget();
    QString getStrNum(); //获取连接码
    void ShowScuessMessage(); //显示成功窗口
    void ShowFailMessage();//显示失败窗口
private:
     Ui::CLoginWidget *ui;
};

#endif // LOGINWIDGET_H
