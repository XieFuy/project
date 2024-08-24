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
    ui->comboBox_2->addItem(QIcon(":/disk.png"),"测试");
    this->mutex = CreateMutex(nullptr,FALSE,nullptr);
    this->localComboBoxPath = "C:\\";

//数据添加测试
//#if 0
//this->showFileInfo("C:\\");
//#endif
    this->initLocalDiskInfo();
    //显示当前路径文件信息
    _beginthreadex(nullptr,0,&CFileOperatorDlg::threadShowFileInfo,this,0,nullptr);

    //进行磁盘切换,显示对应盘符的文件信息
    QObject::connect(ui->comboBox,&QComboBox::currentTextChanged,[=](const QString & str){
        //进行查询对应的磁盘的文件信息
        QString temp = str;
        temp+="\\";
        //清除当前二维表中的行文件信息
        int nRowsCount = this->m_model->rowCount();
        this->m_model->removeRows(0,nRowsCount);
        this->localComboBoxPath = temp;
        _beginthreadex(nullptr,0,&CFileOperatorDlg::threadShowFileInfo,this,0,nullptr);
    });

    //进行点击对应的文件或者文件夹
    QObject::connect(ui->tableView,&QTableView::doubleClicked,[=](const QModelIndex &index){
        if(index.isValid() && index.column() == 0) //确保点击的是文件或者文件夹的名称
        {
          //如果是文件的话就进行运行文件，如果是文件夹的话，就进入对应的子目录，并且comboBox进行显示拼接路径的信息，表格刷新显示子目录的文件信息
          //获取被点击的文件或者文件夹的名称
            // 获取模型
            QString fileName = "";
            const QStandardItemModel *model = qobject_cast<const QStandardItemModel*>(index.model());
            if (model) {
                // 使用DisplayRole获取文本数据
                fileName = model->data(index, Qt::DisplayRole).toString();
            }

            QString fileType = "";
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
             int len = MultiByteToWideChar(CP_UTF8, 0, temp.c_str(), -1, nullptr, 0);
             std::wstring wstr(len, '\0');
             MultiByteToWideChar(CP_UTF8, 0, temp.c_str(), -1, &wstr[0], len);

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

              int index =   ui->comboBox->currentIndex();
              if(index != -1)
              {
                  ui->comboBox->setItemText(index,path);
              }
            }
        }
    });
}

void CFileOperatorDlg::setControlStyleSheet()
{
    ui->pushButton->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;}");
    ui->pushButton_6->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;}");
    ui->pushButton_2->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;}");
    ui->pushButton_7->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;}");
    ui->pushButton_3->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;}");
    ui->pushButton_8->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;}");
    ui->pushButton_4->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;}");
    ui->pushButton_9->setStyleSheet("QPushButton{background-color:rgb(254,254,254);border:none;border-radius:10px;}");
    ui->pushButton_5->setStyleSheet("QPushButton{background-color:rgb(51,103,161);border:none;border-radius:10px;}");
    ui->pushButton_10->setStyleSheet("QPushButton{background-color:rgb(51,103,161);border:none;border-radius:10px;}");
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

//这个操作是一个耗时的操作想办法优化
qint64 CFileOperatorDlg::getDirSize(const QFileInfo& dir) //传递过来一个文件夹 存在bug，只能计算传递过来的文件夹的第一层文件的大小
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
        QStandardItem *nameItem = new QStandardItem(fileInfo.fileName()); //相当于每一个格子的数据
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
