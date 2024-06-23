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
    this->m_Event = CreateEvent(nullptr,FALSE,FALSE,nullptr);
    this->m_CloseEvent = CreateEvent(nullptr,FALSE,FALSE,nullptr);
    this->setMouseTracking(true); //设置MouseTracking鼠标追踪可以即使不按下鼠标键也可以出发MouseMove事件
    ui->label->setAttribute(Qt::WA_TransparentForMouseEvents,true); //设置label的鼠标事件为透明，并且将鼠标事件转移给后面的控件，这样子不会拦截父控件的鼠标移动事件
    this->m_MouseEventMutex = CreateMutex(nullptr,FALSE,nullptr);
    this->m_isMouseMove.store(false);
    this->threshold = 200;
    this->m_timer = new QTimer(this);

    QObject::connect(this,&CWatchDlg::rejected,[=](){
        qDebug()<<"监控对话框被退出";
        delete this;
    });

    QObject::connect(this->m_timer,&QTimer::timeout,[=](){
        qDebug()<<"鼠标停止移动";
        this->PointToClient(this->lastMousePos);
        QPoint remotePoint = this->clientToRemote(this->lastMousePos);
        CMouseEvent mouseEvent;
        mouseEvent.isMoveing = TRUE;
        mouseEvent.x = remotePoint.x();
        mouseEvent.y = remotePoint.y();

        qDebug()<<"isLeft:"<<mouseEvent.isLeftBtn<<"isRight:"<<mouseEvent.isRightBtn
               <<"leftPress:"<<mouseEvent.isLeftBtnPress<<"rightPress:"
               <<mouseEvent.isRightBtnPress<<"leftUp:"<<mouseEvent.isLeftBtnUp
              <<"rightUp:"<<mouseEvent.isRightBtnUp<<"isMoving:"<<mouseEvent.isMoveing
             <<"X:"<<mouseEvent.x<<"Y:"<<mouseEvent.y;

        char buffer[sizeof(CMouseEvent)] = {0};
        memset(buffer,0,sizeof(buffer));
        memcpy(buffer,&mouseEvent,sizeof(CMouseEvent));
        CTestTool::Dump((const BYTE*)buffer,sizeof(CMouseEvent));

        CPacket pack(10,(const BYTE*)buffer,sizeof(buffer));
        CClientSocket* pSocket = CClientSocket::getInstance();
        pSocket->initSocket();
        pSocket->SendPacket(pack);
        WORD ret =  pSocket->DealCommand();
        if(ret == 10)
        {
            qDebug()<<"鼠标移动数据包成功发送";
        }
        pSocket->CloseSocket();
        this->m_isMouseMove.store(false);
        this->m_timer->stop();
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
            this->m_RemoteScreenHeight = image.height();
            this->m_RemoteScreenWidth = image.width();
            ui->label->setPixmap(QPixmap::fromImage(image));
            ui->label->setScaledContents(true);
            SetEvent(this->m_Event);
            ResetEvent(this->m_Event);
        }
    }
}

//左键按下和右键按下
void CWatchDlg::mousePressEvent(QMouseEvent *event)
{
     QPoint point = event->pos();
     if(event->button() == Qt::LeftButton)
     {
         //将坐标转换为
         this->PointToClient(point);


     }
     if(event->button() == Qt::RightButton)
     {
         this->PointToClient(point);
     }

}

//左键弹起和右键弹起
void CWatchDlg::mouseReleaseEvent(QMouseEvent*event)
{



}

//TODO:执行鼠标操作的时候应该进行阻塞控制层的屏幕显示
void CWatchDlg::mouseMoveEvent(QMouseEvent* event)
{    
        qDebug()<<"鼠标移动事件执行！";
        this->lastMousePos = event->pos();
        this->m_isMouseMove.store(true);
        this->m_timer->start(400);
}

void CWatchDlg::PointToClient(QPoint& point)
{
   int y = point.y();
   y -= ui->pushButton->height();
   point.setY(y);
}

CWatchDlg::~CWatchDlg()
{
    //强制结束显示线程//
    TerminateThread(this->m_threadShowScreen,100);
    CloseHandle(this->m_threadShowScreen);
    CloseHandle(this->m_Event);
    this->m_frameIsClosed = TRUE;
    this->m_isFull = FALSE;
    WaitForSingleObject(this->m_CloseEvent,INFINITE);
    delete ui;
}

QPoint CWatchDlg::clientToRemote(QPoint clientPoint)
{
    int clientX = clientPoint.x();
    int clientY = clientPoint.y();

    int remoteX = (this->m_RemoteScreenWidth*clientX)/ui->label->width();
    int remoteY = (this->m_RemoteScreenHeight*clientY)/ui->label->height();

    qDebug()<<"转为远端后的坐标为：("<<remoteX<<","<<remoteY<<")";
    return QPoint(remoteX,remoteY);
}
