#include "tformsernum.h"
#include "ui_tformsernum.h"
#include "headfile.h"
#include "mainwindow.h"
#include <QMessageBox>

TFormSerNum::TFormSerNum(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TFormSerNum)
{
    //设置窗口标志，确保有边框和标题栏、最小化、关闭、最大化
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint |Qt::WindowMaximizeButtonHint);
    ui->setupUi(this);
}

TFormSerNum::~TFormSerNum()
{
    delete ui;
}

void TFormSerNum::displaySerialNumber()
{
    ui->lineEdit_6->setText(batSerNum);
}

void TFormSerNum::on_pushButton_7_clicked()
{
    if (connFlag != CONNECTED)
    {
        QMessageBox::information(this, "警告", "请先建立连接");
        return;
    }
    raise();
    QByteArray byteArray = ui->lineEdit_6->text().toUtf8();  // 转换为 UTF-8 字节数组

    // ===========发送数据===============================
    QByteArray buf(23, 0); // 定义100字节的缓冲区
    buf[0] = MODULE; //地址
    buf[1] = static_cast<quint8>(MASTER_CMD); //主机命令
    buf[2] = SERIAL_NUM_CMD;
    for(quint8 i = 0; i < 20; i++)
    {
        if(i < byteArray.length())
        {
            buf[i + 3] = byteArray[i];
        }else
        {
            buf[i + 3] = 0;
        }
    }
    mainwindow->diyCMDBuild(buf, 23);
}

