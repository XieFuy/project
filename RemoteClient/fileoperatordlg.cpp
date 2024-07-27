#include "fileoperatordlg.h"
#include "ui_cfileoperatordlg.h"

CFileOperatorDlg::CFileOperatorDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CFileOperatorDlg)
{
    ui->setupUi(this);
    this->setWindowTitle(QString("远程文件操作窗口"));
    this->setWindowIcon(QIcon(":/fileTitle.png"));   
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
    ui->comboBox->addItem(QIcon(":/disk.png"),"测试");
    ui->comboBox_2->addItem(QIcon(":/disk.png"),"测试");


//数据添加测试
#if 1
this->showFileInfo("D:\\");
#endif
    // 你可以设置列的宽度、排序等
    ui->tableView->setColumnWidth(0, ui->tableView->width()/2); // 名称列
    ui->tableView->setColumnWidth(1, ui->tableView->width()/8); // 大小列
    ui->tableView->setColumnWidth(2, ui->tableView->width()/8); // 类型
    ui->tableView->setColumnWidth(3,ui->tableView->width()/4);//修改时间
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
            totalSize += getDirSize(dirInfo);
        }
    }
    return totalSize;
}

void CFileOperatorDlg::showFileInfo(QString path)
{
    QStandardItemModel* model = new QStandardItemModel();
    model->setHorizontalHeaderLabels(QStringList()<<"名称"<<"大小"<<"类型"<<"修改时间"); //给模型添加表头列名

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
            qint64 size = this->getDirSize(fileInfo);  //获取当前文件夹的大小

            if (size < 1024) {
                sizeStr = QString::number(size) + " B";
            } else if (size < 1024 * 1024) {
                sizeStr = QString::number(size / 1024.0) + " KB";
            } else if(size < 1024 *1024*1024){
                sizeStr = QString::number(size / (1024.0 * 1024.0)) + " MB";
            }else
            {
                sizeStr = QString::number(size / (1024.0 * 1024.0 * 1024.0)) + " GB";
            }
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
        model->appendRow(row);
    }

    ui->tableView->setModel(model);
}

CFileOperatorDlg::~CFileOperatorDlg()
{
    delete ui;
}
