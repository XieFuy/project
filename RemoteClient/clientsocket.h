#ifndef CCLIENTSOCKET_H
#define CCLIENTSOCKET_H
#include<windows.h>
#include<QDebug>

//网络模块 饿汉式单例设计模式
class CClientSocket
{
public:
    static CClientSocket* getInstance();
private:
    SOCKET m_sockClient;
    BOOL initSocketEnv(); //初始化套接字环境
    BOOL initSocket(); //初始化套接字
    void CloseSocket(); //关闭套接字
    WORD DealCommand(); //处理服务端发送过来的包数据 未实现
    static CClientSocket* m_instance;
    CClientSocket();
    ~CClientSocket();
    CClientSocket(const CClientSocket& clientSocket);
    CClientSocket& operator=(const CClientSocket& clientSocket);
    static void releaseInstance();
    class CHelper{
    public:
        CHelper()
        {
            CClientSocket::getInstance();
        }
        ~CHelper()
        {
            CClientSocket::releaseInstance();
        }
    };
    static CHelper m_helper;
};

#endif // CCLIENTSOCKET_H
