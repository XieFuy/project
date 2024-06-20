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
    memset(&this->m_sockClientAddr,0,sizeof(SOCKADDR_IN));
    this->m_sockClientAddr.sin_port = htons(9527);
    this->m_sockClientAddr.sin_family = AF_INET;
    this->m_sockClientAddr.sin_addr.S_un.S_addr = inet_addr("192.168.232.128"); //服务端的ip地址
//    this->m_sockClientAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
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

WORD CClientSocket::DealCommand()   //应该是这里的效率问题
{
    //TODO:实现处理服务端发送的数据
//    this->m_recvBuffer.clear();
//    this->m_recvBuffer.resize(102400);
    char* recvBuffer = new char[1024000];
    memset(recvBuffer,0,sizeof(recvBuffer));

    //每次接收1024个字节
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
  CTestTool::Dump((const BYTE*)&packet,packet.getDataLenght()+6);
  return send(this->m_sockClient,(const char*)&packet,packet.getDataLenght()+6,0);
}

BOOL CClientSocket::ConnectToServer()
{
    if((connect(this->m_sockClient,(SOCKADDR*)&this->m_sockClientAddr,sizeof(SOCKADDR))) == INVALID_SOCKET)
    {
        qDebug()<<"客户端连接错误："<<__FILE__<<__LINE__<<__FUNCTION__;
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


CClientSocket* CClientSocket::m_instance = nullptr;
CClientSocket::CHelper CClientSocket::m_helper;
