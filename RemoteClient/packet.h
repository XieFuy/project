#ifndef CPACKET_H
#define CPACKET_H
#include<windows.h>
#include<string>
//数据包类，设计数据的自定义协议
class CPacket
{
public:
    CPacket();
    CPacket(WORD cmd,const BYTE* pData,size_t dataSize); //封包(传入数据内容和数据的长度)
    CPacket(const BYTE* pPacketData,size_t& nSize);//解包(传入整个包数据的内容，和整个包的长度)
    CPacket(const CPacket& packet);
    CPacket& operator=(const CPacket& packet);
    WORD getCmd();
    DWORD getDataLenght(); //获取包的数据部分的长度
private:
    WORD m_head; //两个字节的包头
    DWORD m_packetDataLenght; //四字节的包数据的长度
    WORD m_cmd; //两字节命令
    std::string data;//包数据
    WORD m_Sum; //和校验
};
#endif // CPACKET_H
