#include "cclientcontorler.h"
#include<QDebug>

//控制层类构造函数
CClientContorler::CClientContorler()
{
    qDebug()<<"控制层对象成功实例化！";
}

CClientContorler*  CClientContorler::getInstances()
{
    if(CClientContorler::m_instanse == nullptr)
    {
        CClientContorler::m_instanse = new CClientContorler();
    }
    return CClientContorler::m_instanse;
}

void CClientContorler::invokeContorler(Widget& w)
{
    w.show();
}

void CClientContorler::releaseInstances()
{
    if(CClientContorler::m_instanse != nullptr)
    {
        delete CClientContorler::m_instanse;
    }
}

CClientContorler::CClientContorler(const CClientContorler& instances)
{
    if(&instances == this)
    {
        return ;
    }
    this->m_mainWidget = instances.m_mainWidget;
    CClientContorler::m_instanse = instances.m_instanse;
}

CClientContorler& CClientContorler::operator=(const CClientContorler& instances)
{
    if(&instances == this)
    {
        return *this;
    }
    CClientContorler::m_instanse = instances.m_instanse;
    this->m_mainWidget = instances.m_mainWidget;
}

CClientContorler::~CClientContorler()
{
    if(this->m_mainWidget != nullptr)
    {
        delete this->m_mainWidget;
        this->m_mainWidget = nullptr;
    }
    if(this->m_a != nullptr)
    {
        delete this->m_a;
        this->m_a = nullptr;
    }
}

Widget& CClientContorler::getMainWidget()
{
    return *(this->m_mainWidget);
}

void CClientContorler::CreateApplication(int argc,char* argv[])
{
    this->m_a = new QApplication(argc,argv);
    this->m_mainWidget = new Widget();
}

QApplication& CClientContorler::getApplication()
{
    return *(this->m_a);
}

BOOL CClientContorler::ConnectTest(QString strNum)
{
    BOOL ret  = FALSE;
    if(strNum == "000")
    {
        CClientSocket* pClient = CClientSocket::getInstance();
        ret  =  pClient->ConnectTest();
    }
    return ret;
}

void CClientContorler::WatchScreen(QString strNum)
{
    //生成模态对话框对象

    //进行生成一个线程进行向服务端发送数据请求

    this->m_watchDlg = new CWatchDlg();
    _beginthreadex(nullptr,0,&CClientContorler::threadEntrySendWatchPacket,nullptr,0,nullptr);
    this->m_watchDlg->exec();
}

unsigned WINAPI CClientContorler::threadEntrySendWatchPacket(LPVOID arg)
{
    CClientContorler* pCtl = CClientContorler::getInstances();
    pCtl->threadSendWatchPacket();
    _endthreadex(0);
    return 0;
}

void CClientContorler::threadSendWatchPacket()
{
    CClientSocket* pClient = CClientSocket::getInstance();
    Sleep(100);  //休眠进行界面先进行显示
    while(this->m_watchDlg->m_frameIsClosed == FALSE)
    {
        BOOL ret =  pClient->initSocket();
        if(!ret)
        {
            qDebug()<<"套接字初始化错误"<<__FILE__<<__LINE__<<__FUNCTION__<<"错误码："<<WSAGetLastError();
            return ;
        }
        pClient->SendPacket(CPacket(7,nullptr,0));
        pClient->DealCommand();
        this->m_watchDlg->m_ScreenImageDataBuf.resize(pClient->getPacket().getData().size());
        memset(this->m_watchDlg->m_ScreenImageDataBuf.data(),0,this->m_watchDlg->m_ScreenImageDataBuf.size());
        memcpy(this->m_watchDlg->m_ScreenImageDataBuf.data(),pClient->getPacket().getData().c_str(),pClient->getPacket().getData().size()); //这里进行了共享资源的访问，需要进行互斥
        this->m_watchDlg->m_isFull = TRUE;//这里进行了共享资源的访问，需要进行互斥锁
        pClient->CloseSocket();
        SetEvent(this->m_watchDlg->m_Event);
        ResetEvent(this->m_watchDlg->m_Event);
        WaitForSingleObject(this->m_watchDlg->m_Event,INFINITE);
    }
}


CClientContorler* CClientContorler::m_instanse = nullptr;
CClientContorler::CHelper CClientContorler::m_helper;
