#ifndef CMOUSEEVENT_H
#define CMOUSEEVENT_H

#include <QObject>
#include<windows.h>


class CMouseEvent
{
//    Q_OBJECT
public:
    CMouseEvent();
    ~CMouseEvent();
    BOOL isLeftBtn;
    BOOL isRightBtn;
    BOOL isLeftBtnPress;
    BOOL isRightBtnPress;
    BOOL isLeftBtnUp;
    BOOL isRightBtnUp;
    BOOL isMoveing;
    int x;
    int y;
};
#endif // CMOUSEEVENT_H
