#include "loginwidget.h"
#include "ui_loginwidget.h"

CLoginWidget::CLoginWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CLoginWidget)
{
    ui->setupUi(this);
    //进行连接测试
    QObject::connect(ui->pushButton,&QPushButton::clicked,[=](){
        if(ui->lineEdit_2->text()=="000")
        {
            BOOL ret = FALSE;
            CClientContorler* pCtl = CClientContorler::getInstances();
            ret =  pCtl->ConnectTest("000");
            if(ret)
            {
                this->ShowScuessMessage();
            }else
            {
                this->ShowFailMessage();
            }

        }else
        {
         //进行远程桌面
         if(ui->radioButton->isChecked())
         {
             qDebug()<<"选择的是远程桌面";
             //将控制层的代码扔到
             CClientContorler* pCtl = CClientContorler::getInstances();
             pCtl->WatchScreen(ui->lineEdit_2->text());
         }
         //进行远程文件
         if(ui->radioButton_2->isChecked())
         {
             qDebug()<<"选择的是远程文件";
         }
        }
    });
}

void CLoginWidget::ShowScuessMessage()
{

    QMessageBox* scuessMessgaeBox = new QMessageBox(QMessageBox::Information,"连接测试","连接测试成功！",QMessageBox::Ok);
    scuessMessgaeBox->exec();
    //进行点击确认后进行释放窗口的资源
    delete scuessMessgaeBox;
    scuessMessgaeBox = nullptr;
}

void CLoginWidget::ShowFailMessage()
{
    QMessageBox* failMessageBox = new QMessageBox(QMessageBox::Warning,"连接测试","连接测试失败!请重新测试连接。",QMessageBox::Ok);
    failMessageBox->exec();
    delete failMessageBox;
    failMessageBox = nullptr;
}

QString CLoginWidget::getStrNum()
{
    return ui->lineEdit_2->text();
}

CLoginWidget::~CLoginWidget()
{
    delete ui;
}
