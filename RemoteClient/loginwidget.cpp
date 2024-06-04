#include "loginwidget.h"
#include "ui_loginwidget.h"

CLoginWidget::CLoginWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CLoginWidget)
{
    ui->setupUi(this);

    //进行连接测试
    QObject::connect(ui->pushButton,&QPushButton::clicked,[=](){
        CClientContorler* pCtl = CClientContorler::getInstances();
        BOOL ret =  pCtl->ConnectTest(ui->lineEdit_2->text());
        if(ret)
        {
            QMessageBox* box = new QMessageBox(QMessageBox::Information,"连接测试","连接测试成功！",QMessageBox::Ok);
            box->exec();
            //进行点击确认后进行释放窗口的资源
            delete box;
            box = nullptr;
        }else
        {
            QMessageBox* box = new QMessageBox(QMessageBox::Warning,"连接测试","连接测试失败!",QMessageBox::Ok);
            box->exec();
            delete box;
            box = nullptr;
        }
    });
}

CLoginWidget::~CLoginWidget()
{
    delete ui;
}
