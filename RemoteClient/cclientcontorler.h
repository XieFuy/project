#ifndef CCLIENTCONTORLER_H
#define CCLIENTCONTORLER_H

#include"widget.h"
#include<QApplication>
#include<windows.h>
#include"clientsocket.h"
#include"packet.h"
#include"watchdlg.h"
#include"fileoperatordlg.h"
#include<string>

class CFileOperatorDlg;
class Widget;

//控制层，将该类设计成饿汉式单例设计模式
class CClientContorler
{
public:
    static CClientContorler* getInstances(); //获取实例函数
    static void invokeContorler(Widget& w); //启动控制层
    Widget& getMainWidget();
    void CreateApplication(int argc,char* argv[]);
    QApplication& getApplication();
    void sendCommmandPacket(WORD cmd,const BYTE* pData,size_t nSize,BOOL isAutoClosed); //发送数据接口
    BOOL ConnectTest(QString strNum); //测试连接，参数传入识别码
    void WatchScreen(QString strNum); //监控画面
    static unsigned WINAPI threadEntrySendWatchPacket(LPVOID arg); //发送数据包入口线程函数
    void threadSendWatchPacket();
    void SendMouseEventPacket(QPoint point); //进行鼠标操作请求指令发送  
    void RemoteFileOperator(QString num); //进行远程文件操作
    std::string getRemoteDiskInfo(); //进行获取远程
    QVector<QStringList> getRemoteFileInfo(QString currentPath);
    WORD remoteRunFile(std::string& data);
    WORD deleteFile(std::string& data);
    CPacket downLoadFileFromRemote(std::string& data); //本地主机从远程主机下载文件
private:
    CWatchDlg* m_watchDlg;
    QApplication* m_a;
    Widget* m_mainWidget; //主界面
    CFileOperatorDlg* m_fileOperatorDlg = nullptr;//远程文件操作界面
    static CClientContorler* m_instanse; //实例对象指针
    static void releaseInstances(); //释放实例对象
    CClientContorler();
    CClientContorler(const CClientContorler& instances);
    CClientContorler& operator=(const CClientContorler& instances);
    ~CClientContorler();
    class CHelper
    {
    public:
        CHelper()
        {
            CClientContorler::getInstances();
        }
        ~CHelper()
        {
            CClientContorler::releaseInstances();
        }
    };
    static CHelper m_helper;
};

#endif // CCLIENTCONTORLER_H
