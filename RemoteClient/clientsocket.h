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
    size_t SendPacketMouseEvent(CPacket& packet);
    WORD DealCommand(); //处理服务端发送过来的包数据
    WORD DealCommandMouseEvent();
    void CloseSocket(); //关闭套接字
    void CloseSocketMouseEvent();
    CPacket& getPacket();
    BOOL initSocket(); //初始化套接字
    BOOL initSocketMouseEvent();
    BOOL ConnectTest();
    std::string getRemoteDiskInfo();
    QVector<QStringList> getRemoteFileInfo(QString currentPath);
    WORD remoteRunFile(std::string& data);
    WORD deleteFile(std::string& data);
    CPacket downLoadFileFromRemote(std::string& data,char* packetBuffer = nullptr ,size_t* nSize = nullptr);
    CPacket updataFileToRemote(std::string& data,char* packetBuffer = nullptr,size_t* nSize = nullptr);
    SOCKET& getSocketClient();
private:
    SOCKET m_sockClient;
    SOCKADDR_IN m_sockClientAddr;
    SOCKET m_sockClientMouseEvent;
    SOCKADDR_IN m_sockClientAddrMouseEvent;
    std::vector<char> m_recvBuffer;
    CPacket m_packet;
    CPacket m_packetMouseEvent;
    BOOL ConnectToServer(); //连接服务端
    BOOL ConnectToServerMouseEvent();
    BOOL initSocketEnv(); //初始化套接字环境 
    static CClientSocket* m_instance;
    CClientSocket();
    ~CClientSocket();
    CClientSocket(const CClientSocket& clientSocket);
    CClientSocket& operator=(const CClientSocket& clientSocket);
    static void releaseInstance();
    void RecvMultiPackets(std::list<CPacket>& packets);//一次性接收多个包
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
