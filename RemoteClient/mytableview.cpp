#include "mytableview.h"

MyTableView::MyTableView(QWidget *parent) : QTableView(parent)
{
}


void MyTableView::mousePressEvent(QMouseEvent *event)
{
   if(event->button() == Qt::RightButton)
   {
       emit this->rightButtonPress(event->pos());
   }
   QTableView::mousePressEvent(event);
}
