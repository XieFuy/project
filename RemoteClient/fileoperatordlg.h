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
private:
    qint64 getDirSize(const QFileInfo& dir); //获取文件夹的大小
    QStandardItemModel* m_model = nullptr;
    QStandardItemModel* m_model2 = nullptr;
    QStandardItemModel* m_model3 = nullptr;
    QStandardItemModel* tempModel = nullptr;
    HANDLE mutex;
    QString localComboBoxPath; //硬盘磁盘
    void checkLocalDisk(); //检测获取本地主机的盘符
    static unsigned WINAPI threadCheckLocalDisk(LPVOID arg);//检测获取本地主机盘符的线程函数
    static unsigned WINAPI threadShowFileInfo(LPVOID arg); //显示本地主机文件信息的线程函数
    static unsigned WINAPI threadGetDirSize(LPVOID arg);//获取文件夹大小的线程函数
    static unsigned WINAPI threadShowFerchResult(LPVOID arg);//显示本地的查询结果线程函数
    void setFirstModelAndStyle(); //给显示本地主机文件信息的TableView设置model和列宽
    void setSecondModelAndStyle(); //给显示远程主机文件信息的TableView设置model和列宽
    void setThirdModelAndStyle();//给显示文件下载信息的TableView设置model和列宽
    void initLocalDiskInfo();//初始化获取本地主机的盘符信息
    void setControlStyleSheet();//设置控件样式
    void firstModelClear(); //清除第一个model的所有行数据
    void getFileName(QString& fileName,const QModelIndex& index); //获取表格的文件或者文件夹的名称
    void getFileType(QString& fileType,QString& fileName); //获取文件类型(是文件还是文件夹)
    std::wstring multiBytesToWideChar(std::string& str); //将单字节的字符串转为宽字节字符串
    QString getParentFilePath(QString currentPath); //根据当前目录路径获取父目录
    void setComboBoxPath(QString path); //设置comboBox的路径显示的信息
    QString getMostParentPath(QString currentPath); //根据当前路径，获取该路径的最初的父目录(既回退到盘符路径)
    void showFerchResult(QStandardItemModel* tempModel); //显示本地的查询结果
private:
    Ui::CFileOperatorDlg *ui;
};

#endif // FILEOPERATORDLG_H
