#include "loginwidget.h"
#include "ui_loginwidget.h"

CLoginWidget::CLoginWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CLoginWidget)
{
    ui->setupUi(this);

}

CLoginWidget::~CLoginWidget()
{
    delete ui;
}
