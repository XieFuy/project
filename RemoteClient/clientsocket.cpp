#include "clientsocket.h"

CClientSocket::CClientSocket()
{
   qDebug()<<"网络模块初始化成功";
   this->m_sockClient = INVALID_SOCKET;
}

CClientSocket* CClientSocket::getInstance()
{
    if(CClientSocket::m_instance == nullptr)
    {
        CClientSocket::m_instance = new CClientSocket();
    }
    return CClientSocket::m_instance;
}

CClientSocket::~CClientSocket()
{
    WSACleanup();
}

//拷贝构造
CClientSocket::CClientSocket(const CClientSocket& clientSocket)
{
    if(&clientSocket == this)
    {
        return ;
    }
}

//等号运算符重载
CClientSocket& CClientSocket::operator=(const CClientSocket& clientSocket)
{
    if(&clientSocket == this)
    {
        return *this;
    }
}

void  CClientSocket::releaseInstance()
{
    if(CClientSocket::m_instance != nullptr)
    {
        delete CClientSocket::m_instance;
    }
}

BOOL CClientSocket::initSocketEnv()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("WSAStartup failed with error: %d\n", err);
        return FALSE;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return FALSE;
    }
    return TRUE;
}

BOOL CClientSocket::initSocket()
{
    if(this->m_sockClient != INVALID_SOCKET)
    {
        this->CloseSocket();
    }
    this->m_sockClient = socket(AF_INET,SOCK_STREAM,0);
    if(SOCKET_ERROR == this->m_sockClient)
    {
        qDebug()<<"socket init Error:"<<__FILE__<<__LINE__<<__FUNCTION__;
        return FALSE;
    }
}

void CClientSocket::CloseSocket()
{
    if(this->m_sockClient != INVALID_SOCKET)
    {
        closesocket(this->m_sockClient);
        this->m_sockClient = INVALID_SOCKET;
    }
}

WORD CClientSocket::DealCommand()
{
    //TODO:实现处理服务端发送的数据


}

CClientSocket* CClientSocket::m_instance = nullptr;
CClientSocket::CHelper CClientSocket::m_helper;
