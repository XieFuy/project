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
    //ui->comboBox_2->addItem(QIcon(":/disk.png"),"测试");
    this->mutex = CreateMutex(nullptr,FALSE,nullptr);
    this->localComboBoxPath = "C:\\";
    this->fileName = "";
    this->fileType = "";
//数据添加测试
//#if 0
//this->showFileInfo("C:\\");
//#endif

    //初始化远程主机的盘符信息
    this->initRemoteDiskInfo();

    this->initLocalDiskInfo();
    //显示当前路径文件信息
    _beginthreadex(nullptr,0,&CFileOperatorDlg::threadShowFileInfo,this,0,nullptr);

    //频繁快速进行切换磁盘出现崩溃问题  测试解决崩溃问题，原因是在线程函数没有执行完毕的时候就立马快速切换路径导致，所以保证在每次切换前保证上一个线程函数必须完成后再进行查询信息
    //进行磁盘切换,显示对应盘符的文件信息
    QObject::connect(ui->comboBox,&QComboBox::currentTextChanged,[=](const QString & str){
        //进行查询对应的磁盘的文件信息
        QString temp = str;
        temp+="\\";
        this->firstModelClear();
        this->localComboBoxPath = temp; //触发comboBox的信号执行对应的槽函数
        WaitForSingleObject(this->mutex,INFINITE);
        HANDLE thread = (HANDLE)_beginthreadex(nullptr,0,&CFileOperatorDlg::threadShowFileInfo,this,0,nullptr);
        WaitForSingleObject(thread,INFINITE);
        ReleaseMutex(this->mutex);
    });

    //进行点击对应的文件或者文件夹
    QObject::connect(ui->tableView,&QTableView::doubleClicked,[=](const QModelIndex &index){
        if(index.isValid() && index.column() == 0) //确保点击的是文件或者文件夹的名称
        {
          //如果是文件的话就进行运行文件，如果是文件夹的话，就进入对应的子目录，并且comboBox进行显示拼接路径的信息，表格刷新显示子目录的文件信息
          //获取被点击的文件或者文件夹的名称
            // 获取模型
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

    //直接回到盘符的最初目录
    QObject::connect(ui->pushButton_3,&QPushButton::clicked,[=](){
        QString path = this->getMostParentPath(ui->comboBox->currentText());
        QString temp = path;
        temp +="\\";
        this->localComboBoxPath = temp;
        this->setComboBoxPath(path);
    });

    //刷新当前路劲下的文件信息
    QObject::connect(ui->pushButton_4,&QPushButton::clicked,[=](){
       this->reFlashFileInfo();
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
         this->reFlashFileInfo();

        //将目前该目录下的符合条件的文件信息进行存储到临时的模型中
        QStandardItemModel* tempModel = new QStandardItemModel();
        for(int i = 0 ; i < this->m_model->rowCount();i++)
        {
            QList<QStandardItem*> tempList;
            for(int j = 0 ; j < this->m_model->columnCount();j++ )
            {
               QStandardItem* temp =   this->m_model->item(i,j);
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
        //进行清除原本的模型的数据
        this->firstModelClear();
        //将查询后的结果进行赋值给model
        this->tempModel = tempModel;
        HANDLE thread = (HANDLE)_beginthreadex(nullptr,0,&CFileOperatorDlg::threadShowFerchResult,this,0,nullptr);
        WaitForSingleObject(thread,INFINITE);
        delete this->tempModel;
        this->tempModel = nullptr;
        this->ui->lineEdit->setText("");
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

void CFileOperatorDlg::setComboBoxPath(QString path)
{
    int index =   ui->comboBox->currentIndex();
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
       parentPath = currentPath.left(lastIndex); //进行减一，去除掉双斜杠号，避免导致路径解析的不正确
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

void CFileOperatorDlg::getFileName(QString & fileName,const QModelIndex& index)
{
    const QStandardItemModel *model = qobject_cast<const QStandardItemModel*>(index.model());
    if (model) {
        // 使用DisplayRole获取文本数据
        fileName = model->data(index, Qt::DisplayRole).toString();
    }
}

void CFileOperatorDlg::firstModelClear()  //频繁的切换路径会出现程序崩溃的问题
{
    //清除当前二维表中的行文件信息
    int nRowsCount = this->m_model->rowCount();
    this->m_model->removeRows(0,nRowsCount);
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

    ui->tableView_3->setModel(this->m_model3);
    ui->tableView_3->setColumnWidth(0,ui->tableView_3->width()/14*2);
    ui->tableView_3->setColumnWidth(1,ui->tableView_3->width()/14);
    ui->tableView_3->setColumnWidth(2,ui->tableView_3->width()/14);
    ui->tableView_3->setColumnWidth(3,ui->tableView_3->width()/14*4);
    ui->tableView_3->setColumnWidth(4,ui->tableView_3->width()/14*4);
    ui->tableView_3->setColumnWidth(5,ui->tableView_3->width()/14);
    ui->tableView_3->setColumnWidth(6,ui->tableView_3->width()/14);
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
    //ui->tableView->setModel(this->m_model); //用于更新数据 ,目前不需要
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
