#include "watchdlg.h"
#include "ui_cwatchdlg.h"

CWatchDlg::CWatchDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWatchDlg)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/remoteWatch.png"));
    this->setWindowTitle("屏幕监控对话框");
    this->m_isFull = FALSE;
    this->m_frameIsClosed = FALSE;
    //this->m_Mutex = CreateMutex(nullptr,FALSE,nullptr);
    this->m_Event = CreateEvent(nullptr,FALSE,FALSE,nullptr);
    QObject::connect(this,&CWatchDlg::rejected,[=](){
        qDebug()<<"监控对话框被退出";
        delete this;
    });

    //启动发送监控屏幕数据包线程
   // this->m_threadSendWatchPacket = (HANDLE)_beginthreadex(nullptr,0,&CWatchDlg::threadEntrySendWatchPacket,this,0,nullptr);
    //启动屏幕画面显示线程
    this->m_threadShowScreen = (HANDLE)_beginthreadex(nullptr,0,&CWatchDlg::threadEntryShowScreen,this,0,nullptr);
}


unsigned WINAPI CWatchDlg::threadEntryShowScreen(LPVOID arg)
{
    CWatchDlg* thiz = (CWatchDlg*)arg;
    thiz->threadShowScreen();
    _endthreadex(0);
    return 0;
}

void CWatchDlg::threadShowScreen()
{
    while(this->m_frameIsClosed == FALSE)
    {
        WaitForSingleObject(this->m_Event,INFINITE);
       //TODO:写到这里，下次接着继续完善画面显示的功能
        if(this->m_isFull)
        {         
            qDebug()<<"显示监控屏幕被监控到";
            //进行图片显示
            QByteArray ba(this->m_ScreenImageDataBuf.data(),this->m_ScreenImageDataBuf.size());
            QImage image;
            image.loadFromData(ba,"PNG");
            ui->label->setPixmap(QPixmap::fromImage(image));
            ui->label->setScaledContents(true);
            SetEvent(this->m_Event);
            ResetEvent(this->m_Event);
        }
    }
}

CWatchDlg::~CWatchDlg()
{
    //强制结束显示线程//
    TerminateThread(this->m_threadShowScreen,100);
    CloseHandle(this->m_threadShowScreen);
    CloseHandle(this->m_Event);
    this->m_frameIsClosed = TRUE;
    this->m_isFull = FALSE;
    delete ui;
}
