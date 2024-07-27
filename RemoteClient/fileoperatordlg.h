#ifndef FILEOPERATORDLG_H
#define FILEOPERATORDLG_H

#include <QDialog>
#include<QStandardItemModel>
#include<QDir>
#include<QFileInfoList>
#include<QDateTime>
#include<QDebug>

namespace Ui {
class CFileOperatorDlg;
}

class CFileOperatorDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CFileOperatorDlg(QWidget *parent = 0);
    ~CFileOperatorDlg();
    void showFileInfo(QString path); //显示文件信息
    qint64 getDirSize(const QFileInfo& dir); //获取文件夹的大小
private:
    Ui::CFileOperatorDlg *ui;
};

#endif // FILEOPERATORDLG_H
