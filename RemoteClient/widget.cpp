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
    this->setStyleSheet("QWidget{background-color:rgb(18,18,18)}");


    //添加左侧ToolBar
    this->m_toolBar = new QToolBar(this);
    this->m_toolBar->setOrientation(Qt::Vertical);
    this->m_toolBar->show();
    this->m_toolBar->setGeometry(0,44,249,638);
    this->m_toolBar->setStyleSheet("QToolBar{background-color:rgb(41,41,41)}");

    //添加信息标签
    this->m_infoLabel = new QLabel(this);
    this->m_infoLabel->setText("向日葵远程控制");
    this->m_infoLabel->setStyleSheet("QLabel{background-color:rgb(41,41,41);color:rgb(153,153,153);}");
    this->m_infoLabel->setGeometry(0,0,165,44);
    this->m_infoLabel->setAlignment(Qt::AlignCenter);
    this->m_infoLabel->show();

    //添加间隔
    // 创建一个空的QWidget作为间隔
    QWidget *spacerWidget = new QWidget();
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidget->setMinimumSize(234,this->m_infoLabel->height()+45);
    spacerWidget->setStyleSheet("QWidget{background-color:rgb(41,41,41);}");
    // 添加间隔
    this->m_toolBar->addWidget(spacerWidget);


    //添加更新按钮
    this->m_updateBtn = new QPushButton(this);
    this->m_updateBtn->setText("更新");
    this->m_updateBtn->setGeometry(165,0,this->m_toolBar->width() - this->m_infoLabel->width(),this->m_infoLabel->height());
    this->m_updateBtn->setStyleSheet("QPushButton{background-color:rgb(41,41,41);color:rgb(153,153,153);border:none;}");
    this->m_updateBtn->show();

    //给工具栏添加第一个按钮
    this->m_remoteBtn = new QPushButton("远程协助", this->m_toolBar);
    this->m_remoteBtn->setIcon(QIcon(":/Icon.png"));
    this->m_remoteBtn->setStyleSheet("QPushButton{background-color:rgb(41,41,41); color:rgb(224,224,224); width: 100px; height: 50px;text-align:left;font-size:20px;border: 2px solid rgb(41,41,41);border-radius:10px;} QPushButton:hover{background-color:rgb(53,53,53);border: 0.5px solid rgb(53,53,53);border-radius:10px;width: 100px; height: 50px;text-align:left;font-size:20px;color:rgb(224,224,224);}"); //只能通过qss来设置按钮大小
    this->m_toolBar->addWidget(this->m_remoteBtn);

    //点击第一个按钮，改变样式
    QObject::connect(this->m_remoteBtn,&QPushButton::clicked,[=](){
        this->m_remoteBtn->setIcon(QIcon(":/Icon2.png"));
        this->m_remoteBtn->setStyleSheet("QPushButton{background-color:rgb(53,53,53);border: 0.5px solid rgb(53,53,53);border-radius:10px;width: 100px; height: 50px;text-align:left;font-size:20px;color:rgb(224,224,224);}");
        this->m_deviceListBtn->setStyleSheet("QPushButton{background-color:rgb(41,41,41); color:rgb(224,224,224); width: 100px; height: 50px;text-align:left;font-size:20px;border: 2px solid rgb(41,41,41);border-radius:10px;} QPushButton:hover{background-color:rgb(53,53,53);border: 0.5px solid rgb(53,53,53);border-radius:10px;width: 100px; height: 50px;text-align:left;font-size:20px;color:rgb(224,224,224);}");
        this->m_deviceListBtn->setIcon(QIcon(":/IconDevice.png"));
        this->m_findMoreBtn->setStyleSheet("QPushButton{background-color:rgb(41,41,41); color:rgb(224,224,224); width: 100px; height: 50px;text-align:left;font-size:20px;border: 2px solid rgb(41,41,41);border-radius:10px;} QPushButton:hover{background-color:rgb(53,53,53);border: 0.5px solid rgb(53,53,53);border-radius:10px;width: 100px; height: 50px;text-align:left;font-size:20px;color:rgb(224,224,224);}");
        this->m_findMoreBtn->setIcon(QIcon(":/IconFindMore.png"));
    });

    //添加间隔
    //创建空窗口来作为间隔
    QWidget* spacerWidget2 = new QWidget();
    spacerWidget2->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred); //暂时理解为窗口的大小的改变以设定的方式变化
    spacerWidget2->setMinimumSize(234,22); //设置最小的大小
    spacerWidget2->setStyleSheet("QWidget{background-color:rgb(41,41,41);}");
    this->m_toolBar->addWidget(spacerWidget2);

    //给工具栏添加第二个按钮
    this->m_deviceListBtn = new QPushButton("设备列表",this->m_toolBar);
    this->m_deviceListBtn->setIcon(QIcon(":/IconDevice.png"));
    this->m_deviceListBtn->setStyleSheet("QPushButton{background-color:rgb(41,41,41); color:rgb(224,224,224); width: 100px; height: 50px;text-align:left;font-size:20px;border: 2px solid rgb(41,41,41);border-radius:10px;} QPushButton:hover{background-color:rgb(53,53,53);border: 0.5px solid rgb(53,53,53);border-radius:10px;width: 100px; height: 50px;text-align:left;font-size:20px;color:rgb(224,224,224);}");//使用setstylesheet会覆盖原先的stylesheet
    this->m_toolBar->addWidget(this->m_deviceListBtn);

    //点击第二个菜单栏按钮时，改变样式
    QObject::connect(this->m_deviceListBtn,&QPushButton::clicked,[=](){
        this->m_deviceListBtn->setIcon(QIcon(":/IconDevice2.png"));
        this->m_deviceListBtn->setStyleSheet("QPushButton{background-color:rgb(53,53,53);border: 0.5px solid rgb(53,53,53);border-radius:10px;width: 100px; height: 50px;text-align:left;font-size:20px;color:rgb(224,224,224);}");
        this->m_remoteBtn->setStyleSheet("QPushButton{background-color:rgb(41,41,41); color:rgb(224,224,224); width: 100px; height: 50px;text-align:left;font-size:20px;border: 2px solid rgb(41,41,41);border-radius:10px;} QPushButton:hover{background-color:rgb(53,53,53);border: 0.5px solid rgb(53,53,53);border-radius:10px;width: 100px; height: 50px;text-align:left;font-size:20px;color:rgb(224,224,224);}");
        this->m_remoteBtn->setIcon(QIcon(":/Icon.png"));
        this->m_findMoreBtn->setStyleSheet("QPushButton{background-color:rgb(41,41,41); color:rgb(224,224,224); width: 100px; height: 50px;text-align:left;font-size:20px;border: 2px solid rgb(41,41,41);border-radius:10px;} QPushButton:hover{background-color:rgb(53,53,53);border: 0.5px solid rgb(53,53,53);border-radius:10px;width: 100px; height: 50px;text-align:left;font-size:20px;color:rgb(224,224,224);}");
        this->m_findMoreBtn->setIcon(QIcon(":/IconFindMore.png"));
    });

    //添加间隔
    //创建空窗口作为间隔
    QWidget* spacerWidget3 = new QWidget();
    spacerWidget3->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    spacerWidget3->setMinimumSize(234,22);
    spacerWidget3->setStyleSheet("QWidget{background-color:rgb(41,41,41);}");
    this->m_toolBar->addWidget(spacerWidget3);


    //给工具栏添加第三个按钮
    this->m_findMoreBtn = new QPushButton("发现更多",this->m_toolBar);
    this->m_findMoreBtn->setIcon(QIcon(":/IconFindMore.png"));
    this->m_findMoreBtn->setStyleSheet("QPushButton{background-color:rgb(41,41,41); color:rgb(224,224,224); width: 100px; height: 50px;text-align:left;font-size:20px;border: 2px solid rgb(41,41,41);border-radius:10px;} QPushButton:hover{background-color:rgb(53,53,53);border: 0.5px solid rgb(53,53,53);border-radius:10px;width: 100px; height: 50px;text-align:left;font-size:20px;color:rgb(224,224,224);}");
    this->m_toolBar->addWidget(this->m_findMoreBtn);

    //点击第三个菜单按钮，改变样式
    QObject::connect(this->m_findMoreBtn,&QPushButton::clicked,[=](){
        this->m_findMoreBtn->setIcon(QIcon(":/IconFindMore2.png"));
        this->m_findMoreBtn->setStyleSheet("QPushButton{background-color:rgb(53,53,53);border: 0.5px solid rgb(53,53,53);border-radius:10px;width: 100px; height: 50px;text-align:left;font-size:20px;color:rgb(224,224,224);}");
        this->m_remoteBtn->setStyleSheet("QPushButton{background-color:rgb(41,41,41); color:rgb(224,224,224); width: 100px; height: 50px;text-align:left;font-size:20px;border: 2px solid rgb(41,41,41);border-radius:10px;} QPushButton:hover{background-color:rgb(53,53,53);border: 0.5px solid rgb(53,53,53);border-radius:10px;width: 100px; height: 50px;text-align:left;font-size:20px;color:rgb(224,224,224);}");
        this->m_remoteBtn->setIcon(QIcon(":/Icon.png"));
        this->m_deviceListBtn->setStyleSheet("QPushButton{background-color:rgb(41,41,41); color:rgb(224,224,224); width: 100px; height: 50px;text-align:left;font-size:20px;border: 2px solid rgb(41,41,41);border-radius:10px;} QPushButton:hover{background-color:rgb(53,53,53);border: 0.5px solid rgb(53,53,53);border-radius:10px;width: 100px; height: 50px;text-align:left;font-size:20px;color:rgb(224,224,224);}");
        this->m_deviceListBtn->setIcon(QIcon(":/IconDevice.png"));
    });
}

Widget::~Widget()
{
    delete ui;
}
