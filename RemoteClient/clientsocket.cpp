#include "clientsocket.h"

CClientSocket::CClientSocket()
{
   qDebug()<<"网络模块初始化成功";
   this->m_sockClient = INVALID_SOCKET;
   this->initSocketEnv();
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

QVector<QStringList> CClientSocket::getRemoteFileInfo(QString currentPath)
{
    QVector<QStringList> result;
    std::string temp = currentPath.toUtf8().data();
    size_t ret =  this->SendPacket(CPacket(6,(const BYTE*)temp.c_str(),temp.size()));
    //进行接收数据包
    std::list<CPacket> recvList;
    this->RecvMultiPackets(recvList);
    qDebug()<<recvList.size();

    QString  currentPart ;
    QStringList partList;
    //将从服务端获取到的数据进行解析，按格式进行返回
    for(std::list<CPacket>::iterator pos = recvList.begin();pos != recvList.end();pos++)
    {
        QString str = QString::fromLocal8Bit(pos->getData().c_str()); //含有中文的std::string转为QString需要用fromLocal8Bit进行转换
        qDebug()<<str;
        //进行对每一行的数据进行解析
       for(QString::iterator pos = str.begin();pos != str.end();pos++)
       {
           if(*pos != '-')
           {
               currentPart.append(*pos);
           }else
           {
               partList.append(currentPart);
               currentPart.clear();
           }
       }
       qDebug()<<partList.size();
       result.append(partList);
       partList.clear();
    }
    return result;
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

BOOL CClientSocket::initSocketMouseEvent()
{
    if(this->m_sockClientMouseEvent != INVALID_SOCKET)
    {
        this->CloseSocketMouseEvent();
    }
    this->m_sockClientMouseEvent = socket(AF_INET,SOCK_STREAM,0);
    if(SOCKET_ERROR == this->m_sockClientMouseEvent)
    {
        qDebug()<<"socket init Error:"<<__FILE__<<__LINE__<<__FUNCTION__;
        return FALSE;
    }
    memset(&this->m_sockClientAddrMouseEvent,0,sizeof(SOCKADDR_IN));
    this->m_sockClientAddrMouseEvent.sin_port = htons(8888);
    this->m_sockClientAddrMouseEvent.sin_family = AF_INET;
    this->m_sockClientAddrMouseEvent.sin_addr.S_un.S_addr = inet_addr("192.168.232.128"); //服务端的ip地址
//    this->m_sockClientAddrMouseEvent.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
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
    memset(&this->m_sockClientAddr,0,sizeof(SOCKADDR_IN));
    this->m_sockClientAddr.sin_port = htons(9527);
    this->m_sockClientAddr.sin_family = AF_INET;
  //  this->m_sockClientAddr.sin_addr.S_un.S_addr = inet_addr("192.168.232.128"); //服务端的ip地址
    this->m_sockClientAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    return TRUE;
}

void CClientSocket::CloseSocket()
{
    if(this->m_sockClient != INVALID_SOCKET)
    {
        closesocket(this->m_sockClient);
        this->m_sockClient = INVALID_SOCKET;
    }
}

void CClientSocket::CloseSocketMouseEvent()
{
    if(this->m_sockClientMouseEvent!= INVALID_SOCKET)
    {
        closesocket(this->m_sockClientMouseEvent);
        this->m_sockClientMouseEvent = INVALID_SOCKET;
    }
}

WORD CClientSocket::DealCommandMouseEvent()
{
    //TODO:实现处理服务端发送的数据
    char* recvBuffer = new char[1024000];
    memset(recvBuffer,0,sizeof(recvBuffer));

    //每次接收1024个字节
    size_t alReadlyToRecv = 0;
    size_t stepSize = 102400;
    char* pData = recvBuffer;

    //接收单个数据包的所有数据
    while(true)
    {
        int ret =  recv(this->m_sockClientMouseEvent,pData+alReadlyToRecv,stepSize,0);
        if(ret > 0 )
        {
            alReadlyToRecv += ret;

        }else
        {
            qDebug()<<"num:"<<WSAGetLastError();
            break;
        }
    }
    this->m_packetMouseEvent = CPacket((const BYTE*)recvBuffer,alReadlyToRecv);
    delete  []recvBuffer;
    return this->m_packetMouseEvent.getCmd();
}

WORD CClientSocket::DealCommand()   //应该是这里的效率问题
{
    //TODO:实现处理服务端发送的数据
//    this->m_recvBuffer.clear();
//    this->m_recvBuffer.resize(102400);
    char* recvBuffer = new char[1024000];
    memset(recvBuffer,0,sizeof(recvBuffer));

    //每次接收102400个字节
    size_t alReadlyToRecv = 0;
    size_t stepSize = 102400;
//    char* pData = this->m_recvBuffer.data();
     char* pData = recvBuffer;

    //接收单个数据包的所有数据
    while(true)
    {
        int ret =  recv(this->m_sockClient,pData+alReadlyToRecv,stepSize,0);
        if(ret > 0 )
        {
            alReadlyToRecv += ret;

            //this->m_recvBuffer.resize(alReadlyToRecv + stepSize);
        }else
        {
            qDebug()<<"num:"<<WSAGetLastError();
            break;
        }
    }
//    this->m_packet = CPacket((const BYTE*)this->m_recvBuffer.data(),alReadlyToRecv);
    this->m_packet = CPacket((const BYTE*)recvBuffer,alReadlyToRecv);
    delete  []recvBuffer;
    return this->m_packet.getCmd();
}

void CClientSocket::RecvMultiPackets(std::list<CPacket>& packets)
{
    char* recvBuffer = new char[1024000]; //这样子写必须确保所有的数据包不会超过1MB
    memset(recvBuffer,0,1024000);
    size_t stepSize = 102400;
    size_t alReadyRecv = 0;
    char* pData = recvBuffer;
    //接收完整的单个数据包的步骤
        while(true)
        {
           int ret =   recv(this->m_sockClient,pData + alReadyRecv,stepSize,0);
           if(ret > 0 || alReadyRecv != 0)
           {
             alReadyRecv += ret;
             size_t temp = alReadyRecv;
             this->m_packet = CPacket((const BYTE*)recvBuffer,temp);
             memmove(recvBuffer,recvBuffer+temp,1024000 - temp);
             recvBuffer[1024000 - temp] = '\0';
             alReadyRecv -= temp;
             packets.push_back(this->m_packet);
           }else
           {
               break;
           }
        }
        delete []recvBuffer;
}

size_t CClientSocket::SendPacketMouseEvent(CPacket packet)
{
    BOOL ret =  this->initSocketMouseEvent();
    if(!ret)
    {
        qDebug()<<"初始化套接字错误:"<<__FILE__<<__LINE__<<__FUNCTION__<<"错误码："<<WSAGetLastError();
        return 0;
    }
    ret =  this->ConnectToServerMouseEvent();
    if(!ret)
    {
        qDebug()<<"连接服务端错误:"<<__FILE__<<__LINE__<<__FUNCTION__<<"错误码："<<WSAGetLastError();
        return 0;
    }
    std::string data = "";
    packet.toByteData(data);
    CTestTool::Dump((const BYTE*)data.c_str(),data.size());
    return send(this->m_sockClientMouseEvent,(const char*)data.c_str(),data.size(),0);
}

size_t  CClientSocket::SendPacket(CPacket packet)
{
  BOOL ret =  this->initSocket();
  if(!ret)
  {
      qDebug()<<"初始化套接字错误:"<<__FILE__<<__LINE__<<__FUNCTION__<<"错误码："<<WSAGetLastError();
      return 0;
  }
  ret =  this->ConnectToServer();
  if(!ret)
  {
      qDebug()<<"连接服务端错误:"<<__FILE__<<__LINE__<<__FUNCTION__<<"错误码："<<WSAGetLastError();
      return 0;
  }
  std::string data = "";
  packet.toByteData(data);
  CTestTool::Dump((const BYTE*)data.c_str(),data.size());
  return send(this->m_sockClient,(const char*)data.c_str(),data.size(),0);
}

BOOL CClientSocket::ConnectToServerMouseEvent()
{
    if((connect(this->m_sockClientMouseEvent,(SOCKADDR*)&this->m_sockClientAddrMouseEvent,sizeof(SOCKADDR))) == SOCKET_ERROR)
    {
        qDebug()<<"客户端连接错误："<<__FILE__<<__LINE__<<__FUNCTION__<<WSAGetLastError();
        return FALSE;
    }
    return TRUE;
}

BOOL CClientSocket::ConnectToServer()
{
    if((connect(this->m_sockClient,(SOCKADDR*)&this->m_sockClientAddr,sizeof(SOCKADDR))) == INVALID_SOCKET)
    {
        qDebug()<<"客户端连接错误："<<__FILE__<<__LINE__<<__FUNCTION__<<WSAGetLastError();
        return FALSE;
    }
    return TRUE;
}

 CPacket& CClientSocket::getPacket()
 {
     return this->m_packet;
 }

 BOOL CClientSocket::ConnectTest()
 {
     CPacket packet(1981,nullptr,0);
     size_t ret = this->SendPacket(packet);
     this->DealCommand(); //进行接收服务端回应的数据包
     if(this->getPacket().getCmd() == 1981)
     {
         //提示连接成功
         return TRUE;
     }
     return FALSE;
 }

 std::string CClientSocket::getRemoteDiskInfo()
 {
     CPacket packet(4,nullptr,0);
     size_t ret = this->SendPacket(packet);
     WORD cmd =  this->DealCommand();
     return this->m_packet.getData();
 }

CClientSocket* CClientSocket::m_instance = nullptr;
CClientSocket::CHelper CClientSocket::m_helper;
