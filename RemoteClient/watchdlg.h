#ifndef WATCHDLG_H
#define WATCHDLG_H

#include <QDialog>
#include<QDebug>
#include<windows.h>
#include<process.h>
#include"clientsocket.h"
#include<vector>
#include"packet.h"

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
    std::vector<char> m_ScreenImageDataBuf; //存储图片数据的缓冲区
    HANDLE m_Event;  //使用互斥事件
signals:
    void closeDlg(); //向外发送关闭的信号
private:
    HANDLE m_threadShowScreen; //显示监控画面线程
    static unsigned WINAPI threadEntryShowScreen(LPVOID arg); //显示监控画面入口线程函数
    void threadShowScreen();
    Ui::CWatchDlg *ui;
};

#endif // WATCHDLG_H
