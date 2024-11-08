#include "fileoperatordlg.h"
#include "ui_cfileoperatordlg.h"

struct argList //用作线程的参数结构体
{
    qint64* totalSize;
    const QFileInfo *dirInfo;
    CFileOperatorDlg* thiz;
    argList()
    {
        totalSize = nullptr;
        this->dirInfo = nullptr;
        this->thiz = nullptr;
    }
};

CFileOperatorDlg::CFileOperatorDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CFileOperatorDlg)
{
    ui->setupUi(this);
    this->setWindowTitle(QString("远程文件操作窗口"));
    this->setWindowIcon(QIcon(":/fileTitle.png"));
    this->setFirstModelAndStyle();
    this->setSecondModelAndStyle();
    this->setThirdModelAndStyle();
    this->setControlStyleSheet();
    this->mutex = CreateMutex(nullptr,FALSE,nullptr);
    this->mutex2 = CreateMutex(nullptr,FALSE,nullptr);
    this->mutex3 = CreateMutex(nullptr,FALSE,nullptr);
    this->m_event = CreateEvent(nullptr,FALSE,FALSE,nullptr);
    this->m_event2 = CreateEvent(nullptr,FALSE,FALSE,nullptr);
    this->m_event3 = CreateEvent(nullptr,FALSE,FALSE,nullptr);
    this->localComboBoxPath = "C:\\";
    this->remoteComboBoxPath = "C:\\";
    this->fileName = "";
    this->fileType = "";
//数据添加测试
//#if 0
//this->showFileInfo("C:\\");
//#endif

    //初始化远程主机的盘符信息
    this->initRemoteDiskInfo();
    //初始化本地主机的盘符信息
    this->initLocalDiskInfo();
    //初始化显示远程文件信息
    _beginthreadex(nullptr,0,&CFileOperatorDlg::threadShowRemoteFileInfo,this,0,nullptr);

    //显示当前路径文件信息
    _beginthreadex(nullptr,0,&CFileOperatorDlg::threadShowFileInfo,this,0,nullptr);

    //频繁快速进行切换磁盘出现崩溃问题  测试解决崩溃问题，原因是在线程函数没有执行完毕的时候就立马快速切换路径导致，所以保证在每次切换前保证上一个线程函数必须完成后再进行查询信息
    //进行磁盘切换,显示对应盘符的文件信息
    QObject::connect(ui->comboBox,&QComboBox::currentTextChanged,[=](const QString & str){
        //进行查询对应的磁盘的文件信息
        WaitForSingleObject(this->mutex2,INFINITE);
        QString temp = str;
        temp+="\\";
        this->localComboBoxPath = temp; //触发comboBox的信号执行对应的槽函数
        WaitForSingleObject(this->mutex,INFINITE);
        HANDLE thread = (HANDLE)_beginthreadex(nullptr,0,&CFileOperatorDlg::threadShowFileInfo,this,0,nullptr);
        WaitForSingleObject(thread,INFINITE);
        ReleaseMutex(this->mutex);
        ReleaseMutex(this->mutex2);
    });

    //切换磁盘信息进行更新远程主机的文件信息
    QObject::connect(this->ui->comboBox_2,&QComboBox::currentTextChanged,[=](){
        WaitForSingleObject(this->mutex3,INFINITE);
        QString path = ui->comboBox_2->currentText();
        path += "\\";
        this->remoteComboBoxPath = path;
        HANDLE thread = (HANDLE) _beginthreadex(nullptr,0,&CFileOperatorDlg::threadShowRemoteFileInfo,this,0,nullptr);
        WaitForSingleObject(thread,INFINITE);
        ReleaseMutex(this->mutex3);
    });


    //进行点击对应的远程主机的文件或者文件夹
    QObject::connect(ui->tableView_2,&QTableView::doubleClicked,[=](const QModelIndex& index){
        if(index.isValid() && index.column() == 0) //确保点击有效并且点击的是第一列
        {
            //获取被双击的文件/文件夹名
            QString fileName="";
            this->getFileName(fileName,index);
            //获取被双击的文件的类型(文件/文件夹)
            QString fileType = "";
            this->getRemoteFileType(fileType,fileName);

            //根据不同的文件类型来进行不同的对应操作
            if(fileType=="文件")
            {
             //TODO:实现运行文件
             QString filePath = ui->comboBox_2->currentText();
             filePath += "\\";
             filePath += fileName;
             std::string temp =  filePath.toUtf8().data();
             qDebug()<<temp.c_str();
             //将单字节转为多字节
             //std::wstring wstr = this->multiBytesToWideChar(temp);
             //进行发包给服务端进行执行运行文件
             CClientContorler* pCtrl = CClientContorler::getInstances();
             WORD ret  =   pCtrl->remoteRunFile(temp);
             if(ret == 1)
             {
                 //进行提示文件已经执行。
                 QMessageBox* box = new QMessageBox(QMessageBox::Information,"提示","文件已运行",QMessageBox::Ok);
                 box->exec();
                 delete box;
             }
            }else if(fileType == "文件夹")
            {
                //TODO:实现进入子目录
                //对comboBox进行更新
                QString path = ui->comboBox_2->currentText();
                path +="\\";
                path += fileName;
                qDebug()<<path;
                this->setRemoteComboBoxPath(path);
            }
        }
    });

    //进行点击对应的文件或者文件夹
    QObject::connect(ui->tableView,&QTableView::doubleClicked,[=](const QModelIndex &index){
        if(index.isValid() && index.column() == 0) //确保点击的是文件或者文件夹的名称
        {
          //如果是文件的话就进行运行文件，如果是文件夹的话，就进入对应的子目录，并且comboBox进行显示拼接路径的信息，表格刷新显示子目录的文件信息
          //获取被点击的文件或者文件夹的名称
            // 获取文件名称
            QString fileName = "";
            this->getFileName(fileName,index);

            QString fileType = "";
            this->getFileType(fileType,fileName);
            //根据不同的文件类型来进行不同的对应操作
            if(fileType=="文件")
            {
             //TODO:实现运行文件
             QString filePath = ui->comboBox->currentText();
             filePath += "\\";
             filePath += fileName;
             std::string temp =  filePath.toUtf8().data();
             qDebug()<<temp.c_str();

             //将单字节转为多字节
             std::wstring wstr = this->multiBytesToWideChar(temp);
             //包含中文的需要使用多字节
             HINSTANCE ret =  ShellExecuteW(nullptr,nullptr,wstr.data(),nullptr,nullptr,SW_SHOWNORMAL); //找到打不开文件的原因，中文字符为宽字节，需要进行使用宽字节的api
             qDebug()<<ret<<GetLastError();
            }else if(fileType == "文件夹")
            {
                //TODO:实现进入子目录
                //对comboBox进行更新
                QString path = ui->comboBox->currentText();
                path +="\\";
                path += fileName;
                this->setComboBoxPath(path);
            }
        }
    });

    //获取选中的文件/文件夹的文件名和文件类型
    QObject::connect(ui->tableView,&MyTableView::clicked,[=](const QModelIndex &index){
       this->getFileName(this->fileName,index);
       this->getFileType(this->fileType,this->fileName);
       this->getFileSize(this->fileSize,this->fileName);

    });

    QObject::connect(ui->tableView_2,&MyTableView::clicked,[=](const QModelIndex &index){
        this->getFileName(this->fileName,index);
        this->getRemoteFileType(this->fileType,this->fileName);
        this->getRemoteFileSize(this->fileSize,this->fileName);
    });

    //远程主机返回上一级目录
    QObject::connect(this->ui->pushButton_6,&QPushButton::clicked,[=](){
        QString currentPath = this->ui->comboBox_2->currentText();
        qDebug()<<currentPath;
        QString parentPath =  this->getParentFilePath(currentPath);
        qDebug()<<parentPath;
        if(parentPath != "")
        {
         QString temp = parentPath;
         temp +="\\";
         this->remoteComboBoxPath = temp;
         //更改目录路径显示
         this->setRemoteComboBoxPath(parentPath);
        }
    });

    //返回上一级目录
    QObject::connect(ui->pushButton,&QPushButton::clicked,[=](){
        QString currentPath = this->ui->comboBox->currentText();
        qDebug()<<currentPath;
        QString parentPath =  this->getParentFilePath(currentPath);
        qDebug()<<parentPath;
        if(parentPath != "")
        {
         QString temp = parentPath;
         temp +="\\";
         this->localComboBoxPath = temp;
         //更改目录路径显示
         this->setComboBoxPath(parentPath);
        }
    });

    //远程主机返回上一级目录
    QObject::connect(ui->pushButton_7,&QPushButton::clicked,[=](){
        QString currentPath = this->ui->comboBox_2->currentText();
        qDebug()<<currentPath;
        QString parentPath =  this->getParentFilePath(currentPath);
        qDebug()<<parentPath;
        if(parentPath != "")
        {
         QString temp = parentPath;
         temp +="\\";
         this->remoteComboBoxPath = temp;
         //更改目录路径显示
         this->setRemoteComboBoxPath(parentPath);
        }
    });

    //返回上一级目录
    QObject::connect(ui->pushButton_2,&QPushButton::clicked,[=](){
        QString currentPath = this->ui->comboBox->currentText();
        qDebug()<<currentPath;
        QString parentPath =  this->getParentFilePath(currentPath);
        qDebug()<<parentPath;
        if(parentPath != "")
        {
         QString temp = parentPath;
         temp +="\\";
         this->localComboBoxPath = temp;
         //更改目录路径显示
         this->setComboBoxPath(parentPath);
        }
    });

    //远程主机直接回到盘符的最初目录
    QObject::connect(ui->pushButton_8,&QPushButton::clicked,[=](){
        QString path = this->getMostParentPath(ui->comboBox_2->currentText());
        QString temp = path;
        temp +="\\";
        this->remoteComboBoxPath = temp;
        this->setRemoteComboBoxPath(path);
    });

    //直接回到盘符的最初目录
    QObject::connect(ui->pushButton_3,&QPushButton::clicked,[=](){
        QString path = this->getMostParentPath(ui->comboBox->currentText());
        QString temp = path;
        temp +="\\";
        this->localComboBoxPath = temp;
        this->setComboBoxPath(path);
    });

    //刷新远程主机当前路径下的文件信息
    QObject::connect(ui->pushButton_9,&QPushButton::clicked,[=](){
        this->flashRemoteFileInfo();
    });

    //刷新当前路劲下的文件信息
    QObject::connect(ui->pushButton_4,&QPushButton::clicked,[=](){
       this->reFlashFileInfo();
    });


    //模糊查询远程主机下的文件
    QObject::connect(ui->lineEdit_2,&QLineEdit::editingFinished,[=](){
        //查询本地的就可以
        QString text = ui->lineEdit_2->text();
        //根据当前文本显示包含该字段的文件/文件夹信息
        if(text == "")
        {
            return ;
        }

        //先重新加载当前目录下的文件
        //this->flashRemoteFileInfo(); //暂时得查询一次刷新一次

        //将目前该目录下的符合条件的文件信息进行存储到临时的模型中
        QStandardItemModel* tempModel = new QStandardItemModel();
        for(int i = 0 ; i < this->m_model2->rowCount();i++)
        {
            QList<QStandardItem*> tempList;
            for(int j = 0 ; j < this->m_model2->columnCount();j++ )
            {
               QStandardItem* temp =   this->m_model2->item(i,j);
               qDebug()<<temp<<"  "<<temp->text();
               if(temp && j == 0) //确保遍历的是第一列,并且文件
               {
                   if(temp->text().contains(text)) //文件夹名称包含关键字
                   {
                       QStandardItem* tempClone = temp->clone();
                       tempList.push_back(tempClone);
                   }else
                   {
                       break;
                   }

               }else //其他列的内容
               {
                  QStandardItem* tempClone = temp->clone();
                  tempList.push_back(tempClone);
               }
            }
            if(tempList.size() > 0)
            {
               tempModel->appendRow(tempList);
            }
        }

        if(tempModel == nullptr)
        {
            return;
        }
        qDebug()<<tempModel->rowCount();
        //进行清除原本的模型的数据
        this->secondModelClear();  //是清除模型数据的问题
        //将查询后的结果进行赋值给model
        this->tempModel = tempModel;
        HANDLE thread = (HANDLE)_beginthreadex(nullptr,0,&CFileOperatorDlg::threadShowRemoteFerchResult,this,0,nullptr);
        WaitForSingleObject(thread,INFINITE);
        delete this->tempModel;
        this->tempModel = nullptr;
        this->ui->lineEdit_2->setText("");
    });


    //模糊查询当前路径下的文件
    QObject::connect(ui->lineEdit,&QLineEdit::editingFinished,[=](){
        QString text = ui->lineEdit->text();
        //根据当前文本显示包含该字段的文件/文件夹信息
        if(text == "")
        {
            return ;
        }

        //先重新加载当前目录下的文件
        //this->reFlashFileInfo(); //导致空白行增加的原因是进行刷新的时候去执行清空，导致空白行增加

        //将目前该目录下的符合条件的文件信息进行存储到临时的模型中
        QStandardItemModel* tempModel = new QStandardItemModel();
        for(int i = 0 ; i < this->m_model->rowCount();i++)
        {
            QList<QStandardItem*> tempList;
            for(int j = 0 ; j < this->m_model->columnCount();j++ )
            {
               QStandardItem* temp =   this->m_model->item(i,j);
               qDebug()<<temp<<"  "<<temp->text();
               if(temp && j == 0&& temp->text() != "") //确保遍历的是第一列,并且文件
               {
                   if(temp->text().contains(text)) //文件夹名称包含关键字
                   {
                       QStandardItem* tempClone = temp->clone();
                       tempList.push_back(tempClone);
                   }else
                   {
                       break;
                   }

               }else //其他列的内容
               {
                  QStandardItem* tempClone = temp->clone();
                  tempList.push_back(tempClone);
               }
            }
            if(tempList.size() > 0)
            {
                  tempModel->appendRow(tempList);
            }
        }

        if(tempModel == nullptr)
        {
            return;
        }
        //进行清除原本的模型的数据
        this->firstModelClear(); //导致程序的崩溃就是清除表格引起的
        //将查询后的结果进行赋值给model
        this->tempModel = tempModel;
        HANDLE thread = (HANDLE)_beginthreadex(nullptr,0,&CFileOperatorDlg::threadShowFerchResult,this,0,nullptr);
        WaitForSingleObject(thread,INFINITE);
        delete this->tempModel;
        this->tempModel = nullptr;
        this->ui->lineEdit->setText("");
    });

    //删除远程文件
    QObject::connect(ui->tableView_2,&MyTableView::rightButtonPress,[=](QPoint point){
       if(this->deleteButton != nullptr)
       {
            return ;
       }
       qDebug()<<"x:  "<<point.x()<<"Y: "<<point.y();
       this->deleteButton = new DeleteButton(ui->tableView_2);
       deleteButton->setGeometry(point.x(),point.y()+30,deleteButton->width(),deleteButton->height());
       QObject::connect(this->deleteButton,&DeleteButton::buttonClick,[=](){
           //弹出模态对话框确定是否删除文件
           QMessageBox* box = new QMessageBox(QMessageBox::Warning,"删除文件","您确定将该文件删除吗?",QMessageBox::Ok | QMessageBox::No);
           int ret =  box->exec();
           delete box;
           box = nullptr;
           if(ret == QMessageBox::Ok)
           {
               //将文件删除
               //确保是如果类型是文件
               QString filePath = "";
               filePath = ui->comboBox_2->currentText();
               filePath +="\\";
               filePath += this->fileName;  //点击刷新的时候使用

               qDebug()<<filePath;
               if(this->fileType == "文件")
               {
                  std::string tempPath = filePath.toUtf8().data();
                  //传输给服务器
                  CClientContorler* pCtrl = CClientContorler::getInstances();
                  WORD ret = pCtrl->deleteFile(tempPath);
                  qDebug()<<ret;
                  if(ret == 2)
                  {
                      QMessageBox* box = new QMessageBox(QMessageBox::Information,"提示","文件删除成功！",QMessageBox::Ok);
                      box->exec();
                      delete box;
                      box = nullptr;
                  }
               }else if(this->fileType ==  "文件夹")
               {
                   //提示文件夹不能被删除
                   QMessageBox* box = new QMessageBox(QMessageBox::Information,"提示","文件夹不能被删除",QMessageBox::Ok);
                   box->exec();
                   delete box;
                   box = nullptr;
               }
               //重新刷新显示文件信息
               this->flashRemoteFileInfo();
               //将删除文件按钮进行消除
               delete this->deleteButton;
               this->deleteButton = nullptr;
           }else if(ret == QMessageBox::No)
           {
              //将删除文件按钮进行消除
               delete this->deleteButton;
               this->deleteButton = nullptr;
           }
       });
       deleteButton->exec();
    });


    //删除本地文件
    QObject::connect(ui->tableView,&MyTableView::rightButtonPress,[=](QPoint point){
        if(this->deleteButton != nullptr)
        {
            return ;
        }
       this->deleteButton = new DeleteButton(ui->tableView);
       deleteButton->setGeometry(point.x(),point.y()+30,deleteButton->width(),deleteButton->height());
       QObject::connect(this->deleteButton,&DeleteButton::buttonClick,[=](){
           //弹出模态对话框确定是否退出程序
           QMessageBox* box = new QMessageBox(QMessageBox::Warning,"删除文件","您确定将该文件删除吗?",QMessageBox::Ok | QMessageBox::No);
           int ret =  box->exec();
           delete box;
           box = nullptr;
           if(ret == QMessageBox::Ok)
           {
               //将文件删除
               //确保是如果类型是文件
               QString filePath = "";
               filePath = ui->comboBox->currentText();
               filePath +="\\";
               filePath += this->fileName;
               qDebug()<<filePath;
               if(this->fileType == "文件")
               {
                  std::string tempPath = filePath.toUtf8().data();
                  std::wstring wTempPath = this->multiBytesToWideChar(tempPath);
                  WINBOOL ret =  DeleteFileW(wTempPath.data());
                  qDebug()<<ret;
                  if(ret > 0)
                  {
                      QMessageBox* box = new QMessageBox(QMessageBox::Information,"提示","文件删除成功！",QMessageBox::Ok);
                      box->exec();
                      delete box;
                      box = nullptr;
                  }
               }else if(this->fileType ==  "文件夹")
               {
                   //提示文件夹不能被删除
                   QMessageBox* box = new QMessageBox(QMessageBox::Information,"提示","文件夹不能被删除",QMessageBox::Ok);
                   box->exec();
                   delete box;
                   box = nullptr;
               }
               //重新刷新显示文件信息
               this->reFlashFileInfo();
               //将删除文件按钮进行消除
               delete this->deleteButton;
               this->deleteButton = nullptr;

           }else if(ret == QMessageBox::No)
           {
              //将删除文件按钮进行消除
               delete this->deleteButton;
               this->deleteButton = nullptr;
           }
       });
       deleteButton->exec();
    });

    //本地主机向远程主机进行文件断点传输(暂未实现断点)
    QObject::connect(ui->pushButton_10,&QPushButton::clicked,[=](){

        //如果点击的是文件夹就进行提示文件夹不能进行传输
        if(this->fileType == "文件夹")
        {
            QMessageBox* box = new QMessageBox(QMessageBox::Information,"提示","文件夹不能进行发送！",QMessageBox::Ok);
            box->exec();
            delete box;
            return;
        }

        //获取远程主机的完成的文件路径
        QString filePath = ui->comboBox_2->currentText();
        filePath += "\\";
        filePath += this->fileName;
        qDebug()<<"被下载的文件路径为："<<filePath;

        //弹出本地文件文件对话框选择保存的路径
        QString directory = QFileDialog::getExistingDirectory(this,tr("请选择保存文件的文件夹"),QDir::homePath());
        if(!directory.isEmpty())
        {                              
            //进行文件下载的时候放在子线程中去执行
            this->m_filePath = filePath;
            this->m_directory = directory;
            _beginthreadex(nullptr,0,&CFileOperatorDlg::threadWriteFileData,this,0,nullptr); //子线程进行写的部分
            //阻塞等到客户端接收到所有的数据包后判断
            WaitForSingleObject(this->m_event,INFINITE);
            //权限不足提示
            if(this->m_packet.getCmd() == 100)
            {
                QMessageBox* box = new QMessageBox(QMessageBox::Information,"提示","由于权限不足，无法进行下载该文件",QMessageBox::Ok);
                box->exec();
                delete box;
            }else if(this->m_packet.getCmd() == 3 )
            {
                this->addDownLoadInfo(this->m_filePath,this->m_directory);
            }
            SetEvent(this->m_event2);
            ResetEvent(this->m_event2);

            QTimer* timer = new QTimer(this);
            connect(timer,&QTimer::timeout,[=](){
                if(this->m_progress <= 100.0)
                {
                    QProgressBar* p = this->m_progressBarList.front();
                    double temp = this->m_progress;
                    p->setValue(temp);
                    SetEvent(this->m_event3);
                    ResetEvent(this->m_event3);
                    if(this->m_progress == 100.0)
                    {
                        this->m_progress = 0;
                        this->m_progressBarList.pop_front();
                        timer->stop();
                        delete timer;
                    }
                }
            });
            timer->start(0.0001);
        }
    });

    //本地主机向远程主机进行文件断点传输(暂未实现断点)
    QObject::connect(ui->pushButton_5,&QPushButton::clicked,[=](){
        //如果点击的是文件夹就进行提示文件夹不能进行传输
        if(this->fileType == "文件夹")
        {
            QMessageBox* box = new QMessageBox(QMessageBox::Information,"提示","文件夹不能进行发送！",QMessageBox::Ok);
            box->exec();
            delete box;
            return;
        }

        //进行文件上传的时候放在子线程中去执行
         _beginthreadex(nullptr,0,&CFileOperatorDlg::threadReadFileData,this,0,nullptr);//子线程进行写的部分
        //阻塞等到客户端接收到所有的数据包后判断
        WaitForSingleObject(this->m_event,INFINITE);
        //权限不足提示
        if(this->m_packet.getCmd() == 100)
        {
            QMessageBox* box = new QMessageBox(QMessageBox::Information,"提示","由于权限不足，无法在远程主机的当前目录下上传文件！",QMessageBox::Ok);
            box->exec();
            delete box;
        }else if(this->m_packet.getCmd() == 5 )
        {
            this->addDownLoadInfo(this->m_filePath,this->m_directory,"上传");
        }
        SetEvent(this->m_event2);
        ResetEvent(this->m_event2);

        //刷新进度条
        QTimer* timer = new QTimer(this);
        connect(timer,&QTimer::timeout,[=](){
            if(this->m_progress <= 100.0)
            {
                QProgressBar* p = this->m_progressBarList.front();
                double temp = this->m_progress;
                p->setValue(temp);
                SetEvent(this->m_event3);
                ResetEvent(this->m_event3);
                if(this->m_progress == 100.0)
                {
                    this->m_progress = 0;
                    this->m_progressBarList.pop_front();
                    timer->stop();
                    delete timer;
                }
            }
        });
        timer->start(0.001);
    });
}


void CFileOperatorDlg::getFileSize(QString& fileSize,QString& fileName)
{
    //进行遍历获取文件大小
    for(int i = 0 ; i < this->m_model->rowCount(); i++)
    {
        QStandardItem* row = this->m_model->item(i,0);
        if(row && row->text() == fileName)
        {
            QStandardItem* thirdCloumItem  = this->m_model->item(i,1);
            if(thirdCloumItem)
            {
               fileSize = thirdCloumItem->text();
               qDebug()<<"被点击的"<<fileName<<"的类型为："<<fileSize;
            }
        }
    }
}

void CFileOperatorDlg::readFileData()
{
    //先获取需要下载的文件的字节总数
    QString filePath = ui->comboBox->currentText();
    filePath +="\\";
    filePath += this->fileName;


    qDebug()<<"选择被上传的文件为："<<filePath;

    std::string temp = filePath.toUtf8().data();
    std::wstring wFilePath = this->multiBytesToWideChar(temp);

    FILE* pFile = nullptr;
    pFile = _wfopen(wFilePath.data(),L"rb+");
    if(pFile == nullptr)
    {
        qDebug()<<"文件打开失败！";
        return ;
    }
    fseek(pFile,0,SEEK_END);
    long long fileSize = _ftelli64(pFile);
    fseek(pFile,0,SEEK_SET);

    QString storePath = ui->comboBox_2->currentText();
    storePath +="\\";
    storePath += this->fileName;
    this->m_filePath = filePath;
    this->m_directory = storePath;
    storePath +="#";
    CClientContorler* pCtrl = CClientContorler::getInstances();
    std::string strData = storePath.toUtf8().data();
    this->m_packet =  pCtrl->upDataFileToRemote(strData); //发送保存的文件路径
    SetEvent(this->m_event);
    ResetEvent(this->m_event);
    WaitForSingleObject(this->m_event2,INFINITE);
    if(this->m_packet.getCmd() == 100)
    {
        qDebug()<<"子线程结束：权限不足，子线程上传文件失败！";
        return;
    }

    //进行读文件发送到远程端
    long long alReadySend = 0;
    while(alReadySend < fileSize)
    {
        char* buffer = new char[1024000];
        memset(buffer,0,1024000);
        size_t ret = fread(buffer,1,1024000,pFile);
        qDebug()<<"ret: "<<ret;
        if(ret > 0)
        {
            alReadySend += ret;
            char* packet = new char[ret];
            size_t nSize = ret;
            memset(packet,0,ret);
            memcpy(packet,buffer,ret);
            std::string str = "";
            Sleep(2);
            pCtrl->upDataFileToRemote(str,packet,&nSize);//发的是文件内容
            delete []packet;
//            std::string strData(buffer,ret);
//            qDebug()<<"strlen(buffer): "<<strlen(buffer)<<"std::string长度： "<<strData.size();
            long double tempFileSize = fileSize;
            long double tempAlReadySend = alReadySend;
            this->m_progress =(tempAlReadySend /tempFileSize) * 100;
            WaitForSingleObject(this->m_event3,INFINITE);
        }
        delete[] buffer;
    }
    fclose(pFile);
    qDebug()<<"alReadySend: "<<alReadySend<<" size: "<<fileSize;
    //发送结束数据包
    Sleep(2);
    std::string tempStr = "";
    pCtrl->upDataFileToRemote(tempStr);
}

unsigned WINAPI CFileOperatorDlg::threadReadFileData(LPVOID arg)
{
    CFileOperatorDlg* thiz = (CFileOperatorDlg*)arg;
    thiz->readFileData();
    _endthreadex(0);
    return 0;
}

unsigned WINAPI CFileOperatorDlg::threadWriteFileData(LPVOID arg)
{
    CFileOperatorDlg* thiz = (CFileOperatorDlg*)arg;
    thiz->writeFileData(thiz->m_directory,thiz->m_result,thiz->m_filePath);
    _endthreadex(0);
    return 0;
}

void CFileOperatorDlg::writeFileData(QString& directory,std::list<CPacket>& result,QString& filePath)
{
    //先获取需要下载的文件的字节总数
    std::string data = filePath.toUtf8().data();
    data.push_back('#');
    CClientContorler* pCtrl = CClientContorler::getInstances();
   // memset(&this->m_packet,0,sizeof(this->m_packet));
    this->m_packet =  pCtrl->downLoadFileFromRemote(data);
    SetEvent(this->m_event);
    ResetEvent(this->m_event);
    //阻塞，等到判断对话框进行显示完毕后，再往下走
    WaitForSingleObject(this->m_event2,INFINITE);
    //判断是否权限足够   
    //权限不足提示
    if(this->m_packet.getCmd() == 100)
    {
        qDebug()<<"子线程：文件权限不足！";
        return ;
    }

    long long lenght ;
    long long alReadyRecv = 0;
    //如果传递过来的数据包是文件大小进行获取文件大小
    if(this->m_packet.getCmd() == 3)
    {
        memcpy(&lenght,this->m_packet.getData().c_str(),this->m_packet.getData().size());
        qDebug()<<"文件字节大小为："<<lenght;
    }

    QString tempFilePath = directory;
    tempFilePath += "/";
    tempFilePath += this->fileName;

    std::string temp = tempFilePath.toUtf8().data();
    std::wstring wTemp = this->multiBytesToWideChar(temp);

    FILE* pFile = _wfopen(wTemp.data(),L"wb+");
    if(pFile == nullptr)
    {
        qDebug()<<"文件写入失败:  "<<__FILE__<<__LINE__<<__FUNCTION__<<strerror(errno);
        return ;
    }
    std::string temp2 = filePath.toUtf8().data();
    //进行循环写入文件
    while(alReadyRecv < lenght)
    {
        //进行获取到文件数据后进行个写入文件   
        Sleep(1);
        char* packetBuffer = new char[1024000];
        memset(packetBuffer,0,1024000);
        size_t dataSize = 0;
        pCtrl->downLoadFileFromRemote(temp2,packetBuffer,&dataSize);
        size_t size =   fwrite(packetBuffer,1,dataSize,pFile);
        //this->m_packet =  pCtrl->downLoadFileFromRemote(temp);
//        size_t size =   fwrite(this->m_packet.getData().c_str(),1,this->m_packet.getData().size(),pFile);
        delete []packetBuffer;
        alReadyRecv += size;
        long double tempLenght = lenght;
        long double tempAlreadyRecv = alReadyRecv;
        this->m_progress = (tempAlreadyRecv / tempLenght) * 100;
        WaitForSingleObject(this->m_event3,INFINITE);
    }  
    fclose(pFile);
}

void CFileOperatorDlg::addDownLoadInfo(QString& filePath,QString& directory,QString type)
{
    //向传输列表中添加行
    int rowCount = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(rowCount + 1);
    qDebug()<<rowCount;
    //文件名
    QTableWidgetItem* nameItem = new QTableWidgetItem(this->fileName);
    ui->tableWidget->setItem(rowCount,0,nameItem);

    //进度
    QProgressBar* progressBar = new QProgressBar();
    progressBar->setValue(0);
    this->m_progressBarList.push_back(progressBar);
    ui->tableWidget->setCellWidget(rowCount,1,progressBar);
    //delete progressBar;

    //大小
    QTableWidgetItem* sizeItem = new QTableWidgetItem(this->fileSize);
    ui->tableWidget->setItem(rowCount,2,sizeItem);

    //发送路径
    QTableWidgetItem* sendPath = new QTableWidgetItem(filePath);
    ui->tableWidget->setItem(rowCount,3,sendPath);
    //接收路径
    QTableWidgetItem* recvPath = new QTableWidgetItem(directory);
    ui->tableWidget->setItem(rowCount,4,recvPath);

    //方向
    QTableWidgetItem* direction = new QTableWidgetItem(type);
    ui->tableWidget->setItem(rowCount,5,direction);
    //删除
    QPushButton* btn = new QPushButton();
    btn->setStyleSheet("QPushButton{background-color:rgba(0,0,0,0);} QPushButton:hover{background-color:rgb(221,58,95);}");
    btn->setIconSize(QSize(30,30));
    btn->setIcon(QIcon(":/delete.png"));
    this->m_pushuBtnList.push_back(btn);
    ui->tableWidget->setCellWidget(rowCount,6,btn);
}

void CFileOperatorDlg::getRemoteFileSize(QString& fileSize,QString& fileName)
{
    //进行遍历获取文件大小
    for(int i = 0 ; i < this->m_model2->rowCount(); i++)
    {
        QStandardItem* row = this->m_model2->item(i,0);
        if(row && row->text() == fileName)
        {
            QStandardItem* thirdCloumItem  = this->m_model2->item(i,1);
            if(thirdCloumItem)
            {
               fileSize = thirdCloumItem->text();
               qDebug()<<"被点击的"<<fileName<<"的类型为："<<fileSize;
            }
        }
    }
}

void CFileOperatorDlg::showRemoteFerchResult(QStandardItemModel* tempModel)
{
    for(int i = 0 ; i < tempModel->rowCount() ; i++)
    {
        QList<QStandardItem*> row;
        for(int j = 0 ; j < tempModel->columnCount();j++)
        {
            QStandardItem* item =  tempModel->item(i,j);
            if(item)
            {
                QStandardItem* itemClone = item->clone();
                row.push_back(itemClone);
            }
        }
        this->m_model2->appendRow(row);
    }
}

unsigned WINAPI CFileOperatorDlg::threadShowRemoteFerchResult(LPVOID arg)
{
    CFileOperatorDlg* thiz = (CFileOperatorDlg*)arg;
    thiz->showRemoteFerchResult(thiz->tempModel);
    _endthreadex(0);
    return 0;
}


void CFileOperatorDlg::flashRemoteFileInfo()
{
    QString currentPath = this->ui->comboBox_2->currentText();
    QString temp = currentPath;
    temp+="\\";
    this->remoteComboBoxPath = temp;
    this->setRemoteComboBoxPath("");
    this->setRemoteComboBoxPath(currentPath);
}

unsigned WINAPI CFileOperatorDlg::threadShowRemoteFileInfo(LPVOID arg)
{
    CFileOperatorDlg* thiz = (CFileOperatorDlg*)arg;
    thiz->showRemoteFileInfo();
    _endthreadex(0);
    return 0;
}


void CFileOperatorDlg::analysisDiskInfoStr(std::string diskInfoStr)
{
    for(std::string::iterator pos = diskInfoStr.begin(); pos != diskInfoStr.end(); pos++)
    {
        std::string tempStr ;
        tempStr.push_back(*pos);
        tempStr.push_back(':');
        QString str = tempStr.c_str();
        this->ui->comboBox_2->addItem(QIcon(":/disk.png"),str);
    }
}

void CFileOperatorDlg::initRemoteDiskInfo()
{
     CClientContorler* pCtrl =  CClientContorler::getInstances();
     std::string diskInfo =  pCtrl->getRemoteDiskInfo();
     qDebug()<<"接收到的字符串为："<<diskInfo.data();
     this->analysisDiskInfoStr(diskInfo);
     //设置默认项
     this->ui->comboBox_2->setCurrentIndex(0);
}

void CFileOperatorDlg::reFlashFileInfo()
{
    QString currentPath = this->ui->comboBox->currentText();
    QString temp = currentPath;
    temp+="\\";
    this->localComboBoxPath = temp;
    this->setComboBoxPath("");
    this->setComboBoxPath(currentPath);
}

unsigned WINAPI CFileOperatorDlg::threadShowFerchResult(LPVOID arg)
{
    CFileOperatorDlg* thiz = (CFileOperatorDlg*)arg;
    thiz->showFerchResult(thiz->tempModel);
    _endthreadex(0);
    return 0;
}

void CFileOperatorDlg::showFerchResult(QStandardItemModel* tempModel)
{
   for(int i = 0 ; i < tempModel->rowCount() ; i++)
   {
       QList<QStandardItem*> row;
       for(int j = 0 ; j < tempModel->columnCount();j++)
       {
           QStandardItem* item =  tempModel->item(i,j);
           if(item)
           {
               if(item->column() == 0 && item->text() == "")
               {
                   break;
               }
               QStandardItem* itemClone = item->clone();
               row.push_back(itemClone);
           }
       }
       this->m_model->appendRow(row);
   }
}

QString CFileOperatorDlg::getMostParentPath(QString currentPath)
{
    int firstIndex = currentPath.indexOf("\\"); //获取第一次出现的这个字符的下标
    QString temp = currentPath.left(firstIndex);
    return temp;
}

void CFileOperatorDlg::setRemoteComboBoxPath(QString path)
{
    int index = this->ui->comboBox_2->currentIndex();
    if(index != -1)
    {
        this->ui->comboBox_2->setItemText(index,path);
    }
}

void CFileOperatorDlg::setComboBoxPath(QString path)
{
    int index =  ui->comboBox->currentIndex();
    if(index != -1)
    {
        ui->comboBox->setItemText(index,path);
    }
}

QString CFileOperatorDlg::getParentFilePath(QString currentPath)
{
   QString parentPath = "";
   int lastIndex = currentPath.lastIndexOf("\\");
   if(lastIndex != -1)
   {
       parentPath = currentPath.left(lastIndex);
   }
   return parentPath;
}

std::wstring CFileOperatorDlg::multiBytesToWideChar(std::string& str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(len, '\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
    return wstr;
}

void CFileOperatorDlg::getFileType(QString& fileType,QString& fileName)
{
    //进行遍历获取文件类型(是文件还是文件夹)
    for(int i = 0 ; i < this->m_model->rowCount(); i++)
    {
        QStandardItem* row = this->m_model->item(i,0);
        if(row && row->text() == fileName)
        {
            QStandardItem* thirdCloumItem  = this->m_model->item(i,2);
            if(thirdCloumItem)
            {
               fileType = thirdCloumItem->text();
               qDebug()<<"被点击的"<<fileName<<"的类型为："<<fileType;
            }
        }
    }
}

void CFileOperatorDlg::getRemoteFileType(QString& fileType,QString& fileName)
{
    //进行遍历获取文件类型(是文件还是文件夹)
    for(int i = 0 ; i < this->m_model2->rowCount(); i++)
    {
        QStandardItem* row = this->m_model2->item(i,0);
        if(row && row->text() == fileName)
        {
            QStandardItem* thirdCloumItem  = this->m_model2->item(i,2);
            if(thirdCloumItem)
            {
               fileType = thirdCloumItem->text();
               qDebug()<<"被点击的"<<fileName<<"的类型为："<<fileType;
            }
        }
    }
}


void CFileOperatorDlg::getFileName(QString & fileName,const QModelIndex& index)
{
    const QStandardItemModel *model = qobject_cast<const QStandardItemModel*>(index.model());
    if (model) {
        // 使用DisplayRole获取文本数据
        fileName = model->data(index, Qt::DisplayRole).toString();
    }
}

void CFileOperatorDlg::secondModelClear()
{
    int rowCount = this->m_model2->rowCount();
    this->m_model2->removeRows(0,rowCount);
}

void CFileOperatorDlg::firstModelClear()  //频繁的切换路径会出现程序崩溃的问题
{
    //清除当前二维表中的行文件信息 
    int nRowsCount = this->m_model->rowCount();
    this->m_model->removeRows(0,nRowsCount);
    qDebug()<<this->m_model->rowCount();
}

void CFileOperatorDlg::setControlStyleSheet()
{
    ui->pushButton->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;} QPushButton:hover{background-color:rgb(221,58,95);}");
    ui->pushButton_6->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;} QPushButton:hover{background-color:rgb(221,58,95);}");
    ui->pushButton_2->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;} QPushButton:hover{background-color:rgb(221,58,95);}");
    ui->pushButton_7->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;} QPushButton:hover{background-color:rgb(221,58,95);}");
    ui->pushButton_3->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;} QPushButton:hover{background-color:rgb(221,58,95);}");
    ui->pushButton_8->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;} QPushButton:hover{background-color:rgb(221,58,95);}");
    ui->pushButton_4->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;} QPushButton:hover{background-color:rgb(221,58,95);}");
    ui->pushButton_9->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;} QPushButton:hover{background-color:rgb(221,58,95);}");
    ui->pushButton_5->setStyleSheet("QPushButton{background-color:rgb(51,103,161);border:none;border-radius:10px;}  QPushButton:hover{background-color:rgb(221,58,95);}");
    ui->pushButton_10->setStyleSheet("QPushButton{background-color:rgb(51,103,161);border:none;border-radius:10px;} QPushButton:hover{background-color:rgb(221,58,95);}");
}

void CFileOperatorDlg::initLocalDiskInfo()
{
    //显示本机盘符信息
    HANDLE thread = (HANDLE)_beginthreadex(nullptr,0,&CFileOperatorDlg::threadCheckLocalDisk,this,0,nullptr);
    WaitForSingleObject(thread,INFINITE);
    this->ui->comboBox->setCurrentIndex(0);
}

void CFileOperatorDlg::setThirdModelAndStyle()
{
    //第三个表用到的model
    this->m_model3 = new QStandardItemModel();
    this->m_model3->setHorizontalHeaderLabels(QStringList()<<"名称"<<"进度"<<"大小"<<"发送路径"<<"接收路径"<<"方向"<<"删除");

    ui->tableWidget->setColumnCount(7); //设置表的列
    ui->tableWidget->setHorizontalHeaderLabels(QStringList()<<"名称"<<"进度"<<"大小"<<"发送路径"<<"接收路径"<<"方向"<<"删除");
    ui->tableWidget->setColumnWidth(0,ui->tableWidget->width()/14*2);
    ui->tableWidget->setColumnWidth(1,ui->tableWidget->width()/14);
    ui->tableWidget->setColumnWidth(2,ui->tableWidget->width()/14);
    ui->tableWidget->setColumnWidth(3,ui->tableWidget->width()/14*4);
    ui->tableWidget->setColumnWidth(4,ui->tableWidget->width()/14*4);
    ui->tableWidget->setColumnWidth(5,ui->tableWidget->width()/14);
    ui->tableWidget->setColumnWidth(6,ui->tableWidget->width()/14);
}

void CFileOperatorDlg::setSecondModelAndStyle()
{
    //第二个表用到的model
    this->m_model2 = new QStandardItemModel();
    this->m_model2->setHorizontalHeaderLabels(QStringList()<<"名称"<<"大小"<<"类型"<<"修改时间");

    ui->tableView_2->setModel(this->m_model2);
    ui->tableView_2->setColumnWidth(0,ui->tableView_2->width()/2);
    ui->tableView_2->setColumnWidth(1, ui->tableView_2->width()/8); // 大小列
    ui->tableView_2->setColumnWidth(2, ui->tableView_2->width()/8); // 类型
    ui->tableView_2->setColumnWidth(3,ui->tableView_2->width()/4);//修改时间
}

void CFileOperatorDlg::setFirstModelAndStyle()
{
    this->m_model = new QStandardItemModel();
    this->m_model->setHorizontalHeaderLabels(QStringList()<<"名称"<<"大小"<<"类型"<<"修改时间");

    // 你可以设置列的宽度、排序等   设置第一个gripView的每一列的长宽高
    this->ui->tableView->setModel(this->m_model);

    //必须先添加数据model然后在去修改列宽
    ui->tableView->setColumnWidth(0, ui->tableView->width()/2); // 名称列
    ui->tableView->setColumnWidth(1, ui->tableView->width()/8); // 大小列
    ui->tableView->setColumnWidth(2, ui->tableView->width()/8); // 类型
    ui->tableView->setColumnWidth(3,ui->tableView->width()/4);//修改时间
}


unsigned WINAPI CFileOperatorDlg::threadCheckLocalDisk(LPVOID arg)
{
   CFileOperatorDlg* thiz = (CFileOperatorDlg*)arg;
   thiz->checkLocalDisk();
   _endthreadex(0);
   return 0;
}

void CFileOperatorDlg::showRemoteFileInfo()
{
    CClientContorler* pCtrl = CClientContorler::getInstances();
    QString currentPath = this->remoteComboBoxPath;
    if(this->ui->comboBox_2->currentText() == "")
    {
        return;
    }
    QVector<QStringList> result =  pCtrl->getRemoteFileInfo(currentPath);

    //进行将获取到的数据显示到文件列表中
    //先进行清除列表中的文件信息
    this->secondModelClear();

    //进行给model赋值，显示在tableView中
    for(QVector<QStringList>::iterator pos = result.begin();pos != result.end();pos++)
    {
        qDebug()<<pos->at(0)<<pos->at(1)<<pos->at(2)<<pos->at(3);
        QList<QStandardItem*> row;
        //生成文件名
        QStandardItem* fileNameItem = new QStandardItem(pos->at(0));
        fileNameItem->setFlags(fileNameItem->flags() & ~Qt::ItemIsEditable);
        QString path = ui->comboBox_2->currentText();
        path +="\\";
        path += pos->at(0);
        fileNameItem->setToolTip(path);
        row.append(fileNameItem);

        //生成文件大小
        QStandardItem* fileSizeItem = new QStandardItem(pos->at(1));
        fileSizeItem->setFlags(fileSizeItem->flags() & ~Qt::ItemIsEditable);
        row.append(fileSizeItem);

        //生成文件类型
        QStandardItem* fileTypeItem = new QStandardItem(pos->at(2));
        fileTypeItem->setFlags(fileTypeItem->flags() & ~Qt::ItemIsEditable);
        row.append(fileTypeItem);

        //生成文件访问时间
        QStandardItem* fileAcessTime = new QStandardItem(pos->at(3));
        fileAcessTime->setFlags(fileAcessTime->flags() & ~Qt::ItemIsEditable);
        row.append(fileAcessTime);

        //将一行添加到模型当中
        this->m_model2->appendRow(row);
    }
}


//这个操作是一个耗时的操作想办法优化 暂时没有调用到
qint64 CFileOperatorDlg::getDirSize(const QFileInfo& dir)
{
    qint64 totalSize = 0;
    if(dir.isDir())
    {
        QString path = dir.path() +"/"+dir.fileName()+"/"; //dir.path返回的是当前文件所在的文件夹路径，如果是.filename获取的是文件名称
        QDir dirOpen(path); //相当于切换到指定的目录下
        QFileInfoList fileInfoList = dirOpen.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);

        // 遍历文件夹内的所有文件，并累加它们的大小
        for (const QFileInfo &file : fileInfoList) {
            totalSize += file.size();
        }

        // 遍历文件夹内的所有子文件夹，并递归计算它们的大小
        QFileInfoList dirs = dirOpen.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
       for (const QFileInfo &dirInfo : dirs) {
            //计划在这里进行开启子线程进行计算
            argList* arg = new argList();
            arg->dirInfo = &dirInfo;
            arg->thiz = this;
            arg->totalSize = &totalSize;
           // totalSize += getDirSize(dirInfo);
           HANDLE thread = (HANDLE) _beginthreadex(nullptr,0,&CFileOperatorDlg::threadGetDirSize,arg,0,nullptr);
           WaitForSingleObject(thread,INFINITE);
           delete arg;
        }
    }
    return totalSize;
}

unsigned WINAPI CFileOperatorDlg::threadGetDirSize(LPVOID arg)
{
   argList* args = (argList*)arg;
   *args->totalSize += args->thiz->getDirSize(*args->dirInfo);
}

void CFileOperatorDlg::showFileInfo(QString path)
{
//    QStandardItemModel* model = new QStandardItemModel();
//    model->setHorizontalHeaderLabels(QStringList()<<"名称"<<"大小"<<"类型"<<"修改时间"); //给模型添加表头列名

    if(this->ui->comboBox->currentText() == "")
    {
        return ;
    }
    // 假设我们要列出当前目录下的所有文件和文件夹
    QDir dir(path);
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot); // 排除'.'和'..'  并且只显示文件和文件夹
    QFileInfoList files = dir.entryInfoList();

    // 遍历文件和文件夹，并将它们添加到模型中
    for (const QFileInfo &fileInfo : files) {
        QList<QStandardItem*> row; //代表一行

        // 名称
        QStandardItem *nameItem = new  QStandardItem(fileInfo.fileName()); //相当于每一个格子的数据
        nameItem->setToolTip(fileInfo.filePath()); // 可以设置工具提示为完整路径
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        row.append(nameItem);//将这一格的内容添加到行中

        // 大小
        QString sizeStr;
        if (fileInfo.isFile()) {
            qint64 size = fileInfo.size();  //获取文件大小
            if (size < 1024) {  //如果是1KB以内
                sizeStr = QString::number(size) + " B";
            } else if (size < 1024 * 1024) {  //如果是1MB以内
                sizeStr = QString::number(size / 1024.0) + " KB";
            } else if(size < 1024 *1024*1024){  //如果是1GB以内
                sizeStr = QString::number(size / (1024.0 * 1024.0)) + " MB";
            }else  //比1GB还要大
            {
                sizeStr = QString::number(size / (1024.0 * 1024.0 * 1024.0)) + " GB";
            }
        } else { //如果是文件夹
            //TODO:文件先不显示大小，后续再实现显示大小的功能

            //如果是文件夹先显示正在计算，后再显示真正的大小
//            qint64 size = this->getDirSize(fileInfo);  //获取当前文件夹的大小
//            if (size < 1024) {
//                sizeStr = QString::number(size) + " B";
//            } else if (size < 1024 * 1024) {
//                sizeStr = QString::number(size / 1024.0) + " KB";
//            } else if(size < 1024 *1024*1024){
//                sizeStr = QString::number(size / (1024.0 * 1024.0)) + " MB";
//            }else
//            {
//                sizeStr = QString::number(size / (1024.0 * 1024.0 * 1024.0)) + " GB";
//            }
        }

        QStandardItem *sizeItem = new QStandardItem(sizeStr);
        sizeItem->setFlags(sizeItem->flags() & ~Qt::ItemIsEditable);
        row.append(sizeItem);

        //类型
        QString typeNameStr;
        if (fileInfo.isFile()) {
            typeNameStr = "文件";
        } else {
            typeNameStr = "文件夹";
        }
        QStandardItem *typeNameItem = new QStandardItem(typeNameStr);
        typeNameItem->setFlags(typeNameItem->flags() & ~Qt::ItemIsEditable);
        row.append(typeNameItem);

        // 修改日期
        QStandardItem *dateItem = new QStandardItem(fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss"));
        dateItem->setFlags(dateItem->flags() & ~Qt::ItemIsEditable);
        row.append(dateItem);

        // 将行添加到模型中
        //model->appendRow(row);
        this->m_model->appendRow(row); //可以将一个model理解为一个二维表
    }
}

unsigned WINAPI CFileOperatorDlg::threadShowFileInfo(LPVOID arg)
{
    CFileOperatorDlg* thiz = (CFileOperatorDlg*)arg;
    thiz->firstModelClear();
    thiz->showFileInfo(thiz->localComboBoxPath); //默认加载的C盘
    _endthreadex(0);
    return 0;
}

void CFileOperatorDlg::checkLocalDisk()//非阻塞UI线程，子线程进行添加UI
{
  int i = 0 ;
  for(int i = 1 ; i <= 26; i++)
  {
      if(_chdrive(i) == 0)
      {
          //将每一个盘符进行添加到comboBox中
          QChar str = 'A' + i - 1;
          QString tempStr;
          tempStr.push_back(str);
          tempStr+=":";
          ui->comboBox->addItem(QIcon(":/disk.png"),tempStr);
      }
  }
}

CFileOperatorDlg::~CFileOperatorDlg()
{
    delete ui;
    if(this->m_model != nullptr)
    {
        delete this->m_model;
        this->m_model = nullptr;
    }
    if(this->m_model2 != nullptr)
    {
        delete this->m_model2;
        this->m_model2 = nullptr;
    }
    if(this->m_model3 != nullptr)
    {
        delete this->m_model3;
        this->m_model3 = nullptr;
    }
}
