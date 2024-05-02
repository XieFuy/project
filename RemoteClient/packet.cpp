#include "packet.h"

CPacket::CPacket()
{
    this->m_head = 0xFEFF;
    this->m_packetDataLenght = 0;
    this->m_Sum = 0;
    this->data = "";
}

CPacket::CPacket(WORD cmd,const BYTE* pData,size_t dataSize)
{
    this->m_head = 0xFEFF;
    this->m_cmd = cmd;
    this->m_packetDataLenght = dataSize + 4;
    this->m_Sum = 0;
    if(pData != nullptr)
    {
        memcpy((char*)this->data.c_str(),pData,dataSize);
        //计算和校验
        //一个char类型字符整数，进行& 0xFF的值可以转换为整型的整数
        for(std::string::iterator pos = this->data.begin();pos != this->data.end();pos++)
        {
            this->m_Sum += (((BYTE)(*pos)) & 0xFF);
        }
    }
}

CPacket::CPacket(const BYTE* pPacketData,size_t& nSize)
{
   //记录解包所消耗的字节数
   //TODO:接着下去


}


