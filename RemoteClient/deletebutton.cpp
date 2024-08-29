#include "deletebutton.h"
#include "ui_deletebutton.h"

DeleteButton::DeleteButton(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeleteButton)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    QObject::connect(ui->pushButton,&QPushButton::clicked,[=](){
        emit this->buttonClick();
    });
}

DeleteButton::~DeleteButton()
{
    delete ui;
}
