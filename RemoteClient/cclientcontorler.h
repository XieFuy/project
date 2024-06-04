#ifndef CCLIENTCONTORLER_H
#define CCLIENTCONTORLER_H

#include"widget.h"
#include<QApplication>
#include<windows.h>
#include"clientsocket.h"
#include"packet.h"

class Widget;

//控制层，将该类设计成饿汉式单例设计模式
class CClientContorler
{
public:
    static CClientContorler* getInstances(); //获取实例函数
    static void invokeContorler(Widget& w); //启动控制层
    Widget& getMainWidget();
    void CreateApplication(int argc,char* argv[]);
    QApplication& getApplication();
    void sendCommmandPacket(WORD cmd,const BYTE* pData,size_t nSize,BOOL isAutoClosed); //发送数据接口
    BOOL ConnectTest(QString strNum); //测试连接，参数传入识别码
private:
    QApplication* m_a;
    Widget* m_mainWidget;
    static CClientContorler* m_instanse; //实例对象指针
    static void releaseInstances(); //释放实例对象
    CClientContorler();
    CClientContorler(const CClientContorler& instances);
    CClientContorler& operator=(const CClientContorler& instances);
    ~CClientContorler();
    class CHelper
    {
    public:
        CHelper()
        {
            CClientContorler::getInstances();
        }
        ~CHelper()
        {
            CClientContorler::releaseInstances();
        }
    };
    static CHelper m_helper;
};

#endif // CCLIENTCONTORLER_H
