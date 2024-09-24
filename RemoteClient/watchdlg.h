#ifndef WATCHDLG_H
#define WATCHDLG_H

#include <QDialog>
#include<QDebug>
#include<windows.h>
#include<process.h>
#include"clientsocket.h"
#include<vector>
#include"packet.h"
#include<QMouseEvent>
#include<QPoint>
#include"cmouseevent.h"
#include<atomic>
#include<QTimer>
#include"ctesttool.h"


namespace Ui {
class CWatchDlg;
}

class CWatchDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CWatchDlg(QWidget *parent = 0);
    ~CWatchDlg();
    BOOL m_isFull;//表示缓存中是否还有数据
    BOOL m_frameIsClosed;//TODO:写到这里，接着继续
    char* recvbuffer = nullptr;
    long long bufferSize = 0;
    std::vector<char> m_ScreenImageDataBuf; //存储图片数据的缓冲区
    std::vector<char> m_lastScreenImageDataBuf;//存储上一帧的图片数据缓冲区
    HANDLE m_Event;  //使用互斥事件来进行显示
    HANDLE m_CloseEvent; //监视窗口关闭时的互斥事件
    HANDLE m_MouseEventMutex; //进行处理鼠标和屏幕显示的互斥事件
    void mousePressEvent(QMouseEvent *event) override; //鼠标按下事件
    void mouseReleaseEvent(QMouseEvent *event) override; //鼠标上升事件
    void mouseMoveEvent(QMouseEvent * event) override; //鼠标移动事件
    void PointToClient(QPoint& point);//坐标转换函数
    QPoint lastMousePos;
    int threshold;
    std::atomic<bool> m_isMouseMove;
    QTimer* m_timer; //配合鼠标移动事件判断是否鼠标停止移动
    int m_RemoteScreenWidth; //服务端屏幕的宽
    int m_RemoteScreenHeight; //服务端屏幕的高
    QPoint clientToRemote(QPoint clientPoint);//将本地坐标转换为远端坐标
signals:
    void closeDlg(); //向外发送关闭的信号
private:
    HANDLE m_threadShowScreen; //显示监控画面线程
    static unsigned WINAPI threadEntryShowScreen(LPVOID arg); //显示监控画面入口线程函数
    void threadShowScreen();
    Ui::CWatchDlg *ui;
};

#endif // WATCHDLG_H
