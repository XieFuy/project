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
        this->data.resize(dataSize);
        memcpy((char*)this->data.c_str(),pData,dataSize);
        //计算和校验
        //一个char类型字符整数，进行& 0xFF的值可以转换为整型的整数
       /* for(std::string::iterator pos = this->data.begin();pos != this->data.end();pos++)
        {
            this->m_Sum += (((BYTE)(*pos)) & 0xFF);
        }*/
    }
}

CPacket::CPacket(const BYTE* pPacketData,size_t& nSize)
{
   //找到包头
   size_t i =0;
   for(;i<nSize;i++)
   {
       if(*((WORD*)(pPacketData+i)) == 0xFEFF)
       {
           this->m_head = *((WORD*)pPacketData);
           i+=2;
           break;
       }
   }

   //判断包是否合理
   if((i + 4 + 2 + 2) > nSize)
   {
       nSize = 0;
       return ;
   }

   //存储包长度，并判断数据包的合理
   this->m_packetDataLenght = *((DWORD*)(pPacketData+i));
   i += 4;
   if(this->m_packetDataLenght + i > nSize)
   {
       nSize = 0;
       return ;
   }

   //存储命令，判断数据包是否合理
   this->m_cmd = *((WORD*)(pPacketData+i));
   i += 2;

   //如果包的数据不为空，则进行拷贝
   if((this->m_packetDataLenght - 4) > 0)
   {
       this->data.resize(this->m_packetDataLenght-2-2);
       memcpy((char*)this->data.c_str(),pPacketData+i,this->m_packetDataLenght-2-2);
       i += this->m_packetDataLenght - 2 - 2;
   }

   //存储和校验
   this->m_Sum = *((WORD*)(pPacketData+i));
   i += 2;

  /* WORD sum = 0;
   for(int j = 0;j<this->data.size();j++)
   {
       sum += (((BYTE)this->data.at(j)) & 0xFF);
   }

   if(sum == this->m_Sum)
   {
       nSize = i;
       return ;
   }
   nSize = 0;*/
   return ;
}

CPacket::CPacket(const CPacket& packet)
{
    if(&packet == this)
    {
        return;
    }
    this->m_head = packet.m_head;
    this->m_cmd = packet.m_cmd;
    this->m_packetDataLenght = packet.m_packetDataLenght;
    this->data = packet.data;
    this->m_Sum = packet.m_Sum;
}

CPacket& CPacket::operator=(const CPacket& packet)
{
    this->m_head = packet.m_head;
    this->m_cmd = packet.m_cmd;
    this->m_packetDataLenght = packet.m_packetDataLenght;
    this->data = packet.data;
    this->m_Sum = packet.m_Sum;
}

WORD CPacket::getCmd()
{
    return this->m_cmd;
}

DWORD CPacket::getDataLenght()
{
    return this->m_packetDataLenght;
}

std::string& CPacket::getData()
{
    return this->data;
}

void CPacket::toByteData(std::string& data)
{
    data.resize(this->m_packetDataLenght + 6);
    char* pData = (char*)data.c_str();
    memcpy(pData,&this->m_head,sizeof(this->m_head));
    pData += 2;
    memcpy(pData,&this->m_packetDataLenght,sizeof(this->m_packetDataLenght));
    pData += 4;
    memcpy(pData,&this->m_cmd,sizeof(this->m_cmd));
    pData += 2;
    memcpy(pData,this->data.c_str(),this->data.size());
    pData += this->data.size();
    memcpy(pData, &this->m_Sum, sizeof(this->m_Sum));
    pData += 2;
    qDebug()<<"进行转换后整个包的数据长度为: "<<data.size();
}
