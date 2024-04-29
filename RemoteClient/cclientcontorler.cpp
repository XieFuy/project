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

CClientContorler* CClientContorler::m_instanse = nullptr;
CClientContorler::CHelper CClientContorler::m_helper;
