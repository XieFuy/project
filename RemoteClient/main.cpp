//#include "widget.h"
#include"cclientcontorler.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    //QApplication a(argc, argv);
//    Widget w;
//    w.show();
    CClientContorler* pCtrl = CClientContorler::getInstances();
    pCtrl->CreateApplication(argc,argv);
    CClientContorler::invokeContorler(pCtrl->getMainWidget());
    return pCtrl->getApplication().exec();
    //return a.exec();
}
