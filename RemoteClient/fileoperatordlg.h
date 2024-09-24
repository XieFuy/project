#ifndef FILEOPERATORDLG_H
#define FILEOPERATORDLG_H

#include <QDialog>
#include<QStandardItemModel>
#include<QDir>
#include<QFileInfoList>
#include<QDateTime>
#include<QDebug>
#include<windows.h>
#include<process.h>
#include<direct.h>
#include<string>
#include<shellapi.h>
#include<deletebutton.h>
#include<QMessageBox>
#include<QModelIndex>
#include"cclientcontorler.h"
#include<QFileDialog>
#include<QProgressBar>
#include<list>

namespace Ui {
class CFileOperatorDlg;
}

class CFileOperatorDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CFileOperatorDlg(QWidget *parent = 0);
    ~CFileOperatorDlg();
    void showFileInfo(QString path); //显示文件信息
signals:
    void recvFinsh(); //表示接收完毕所有的数据包
private:
    qint64 getDirSize(const QFileInfo& dir); //获取文件夹的大小
    QStandardItemModel* m_model = nullptr;
    QStandardItemModel* m_model2 = nullptr;
    QStandardItemModel* m_model3 = nullptr;
    QStandardItemModel* tempModel = nullptr;
    HANDLE mutex;
    HANDLE mutex2;
    HANDLE mutex3;
    HANDLE m_event;
    HANDLE m_event2;
    HANDLE m_event3;
    double m_progress; //进度条的进度
    QString localComboBoxPath; //硬盘磁盘
    QString remoteComboBoxPath;
    DeleteButton * deleteButton = nullptr;
    QMessageBox* msg = nullptr;
    QString fileName;
    QString fileType;
    QString fileSize;
    CPacket m_packet;
    std::list<QProgressBar*> m_progressBarList;
    std::list<QPushButton*> m_pushuBtnList;
    QString m_directory;//本地主机存储远程主机的的保存目录
    std::list<CPacket> m_result; //用于接收多个数据包的结果的列表
    QString m_filePath; //用于存储该下载文件的路径
    void checkLocalDisk(); //检测获取本地主机的盘符
    static unsigned WINAPI threadCheckLocalDisk(LPVOID arg);//检测获取本地主机盘符的线程函数
    static unsigned WINAPI threadShowFileInfo(LPVOID arg); //显示本地主机文件信息的线程函数
    static unsigned WINAPI threadGetDirSize(LPVOID arg);//获取文件夹大小的线程函数
    static unsigned WINAPI threadShowFerchResult(LPVOID arg);//显示本地的查询结果线程函数
    static unsigned WINAPI threadShowRemoteFileInfo(LPVOID arg);//显示远程主机文件信息的线程函数
    static unsigned WINAPI threadShowRemoteFerchResult(LPVOID arg);//显示远程主机的查询结果的线程函数
    static unsigned WINAPI threadWriteFileData(LPVOID arg); //子线程去写文件的线程函数
    static unsigned WINAPI threadReadFileData(LPVOID arg);//子线程去读文件的线程函数

    void setFirstModelAndStyle(); //给显示本地主机文件信息的TableView设置model和列宽
    void setSecondModelAndStyle(); //给显示远程主机文件信息的TableView设置model和列宽
    void setThirdModelAndStyle();//给显示文件下载信息的TableView设置model和列宽
    void initLocalDiskInfo();//初始化获取本地主机的盘符信息
    void setControlStyleSheet();//设置控件样式
    void secondModelClear();//清除第二个model的所有行数据
    void firstModelClear(); //清除第一个model的所有行数据
    void getFileSize(QString& fileSize,QString& fileName);//获取文件大小
    void getFileName(QString& fileName,const QModelIndex& index); //获取表格的文件或者文件夹的名称
    void getFileType(QString& fileType,QString& fileName); //获取文件类型(是文件还是文件夹)
    void getRemoteFileSize(QString& fileSize,QString& fileName);//获取远程主机文件大小
    void getRemoteFileType(QString& fileType,QString& fileName);//获取远程主机文件类型
    std::wstring multiBytesToWideChar(std::string& str); //将单字节的字符串转为宽字节字符串
    QString getParentFilePath(QString currentPath); //根据当前目录路径获取父目录
    void setComboBoxPath(QString path); //设置comboBox的路径显示的信息
    void setRemoteComboBoxPath(QString path);//设置远程主机的comboBox的路径显示的信息
    QString getMostParentPath(QString currentPath); //根据当前路径，获取该路径的最初的父目录(既回退到盘符路径)
    void showFerchResult(QStandardItemModel* tempModel); //显示本地的查询结果
    void showRemoteFerchResult(QStandardItemModel* tempModel);//显示远程主机查询结果
    void reFlashFileInfo(); //刷新本地主机当前路径下的文件信息
    void initRemoteDiskInfo(); //初始化远程主机的盘符信息
    void analysisDiskInfoStr(std::string diskInfoStr);//解析盘符信息字符串
    void showRemoteFileInfo(); //显示远程主机当前目录下的文件信息
    void flashRemoteFileInfo(); //进行刷新远程主机的当前目录下的文件信息
    void addDownLoadInfo(QString& filePath,QString& directory,QString type="下载");//添加下载文件的信息到传输列表中
    void writeFileData(QString& directory,std::list<CPacket>& result,QString& filePath); //传入保存目录进行写入文件数据
    void readFileData(); //将本地文件内容读取上传至远端PC机
private:
    Ui::CFileOperatorDlg *ui;
};

#endif // FILEOPERATORDLG_H
