#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QToolBar>
#include<QLabel>
#include<QPushButton>
#include<QAction>
#include<QVBoxLayout>
#include<QSpacerItem>
#include"loginwidget.h"

class CLoginWidget;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;
    QToolBar* m_toolBar = nullptr;
    QLabel* m_infoLabel = nullptr;
    QPushButton* m_updateBtn = nullptr;
    QPushButton* m_remoteBtn = nullptr; //远程协助按钮
    QPushButton* m_deviceListBtn = nullptr; //设备列表按钮
    QPushButton* m_findMoreBtn = nullptr; //发现更多按钮
    CLoginWidget* m_loginWidget = nullptr; //登录界面
};

#endif // WIDGET_H
