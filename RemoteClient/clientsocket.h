#ifndef CCLIENTSOCKET_H
#define CCLIENTSOCKET_H
#include<windows.h>
#include<QDebug>
#include"packet.h"
#include<vector>
#include<string>
#include"ctesttool.h"
//网络模块 饿汉式单例设计模式
class CClientSocket
{
public:
    static CClientSocket* getInstance();
    size_t SendPacket(CPacket packet); //进行发送数据包 返回发送数据包的长度
     WORD DealCommand(); //处理服务端发送过来的包数据 未实现
     void CloseSocket(); //关闭套接字
     CPacket& getPacket();
private:
    SOCKET m_sockClient;
    SOCKADDR_IN m_sockClientAddr;
    std::vector<char> m_recvBuffer;
    CPacket m_packet;
    BOOL ConnectToServer(); //连接服务端
    BOOL initSocketEnv(); //初始化套接字环境
    BOOL initSocket(); //初始化套接字   
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
