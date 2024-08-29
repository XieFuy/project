#ifndef MYTABLEVIEW_H
#define MYTABLEVIEW_H

#include <QTableView> //自定义控件类，这里在提升的时候选择了全局包含，所以不需要再用到的cpp文件中引入头文件，否则则需要在用到的地方引入头文件
#include <QMouseEvent>
#include <QDebug>


class MyTableView : public QTableView
{
    Q_OBJECT
public:
    explicit MyTableView(QWidget *parent = 0);
void mousePressEvent(QMouseEvent *event) override; //鼠标按下事件
signals:
void rightButtonPress(QPoint point); //声明鼠标右键按下的信号
public slots:
};

#endif // MYTABLEVIEW_H
