#include "widget.h"
#include "ui_widget.h"

//进入的首界面类
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/mainIcon.png"));  //路径格式：冒号、前缀名、文件名
    this->setWindowTitle("远程控制客户端");
    this->setFixedSize(997,681);
    this->setStyleSheet("QWidget{background-color:rgba(18,18,18,100)}");
}

Widget::~Widget()
{
    delete ui;
}
