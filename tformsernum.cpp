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

void TFormSerNum::on_pushButton_7_clicked()
{
    if (connFlag != CONNECTED)
    {
        QMessageBox::information(this, "警告", "请先建立连接");
        return;
    }
    raise();
    quint32 serNum = ui->lineEdit_6->text().toInt();
    quint16 serNumL = (serNum & 0xFFFF);
    quint16 serNumH = (serNum >> 16);

    // ===========发送数据===============================
    QByteArray buf(11, 0); // 定义100字节的缓冲区
    buf[0] = MODULE; //地址
    buf[1] = WRITE_MULTI_CMD; //主机命令
    buf[2] = 0x43;
    buf[3] = 0x1D;
    buf[4] = 0x00;
    buf[5] = 0x02;
    buf[6] = 0x04;
    buf[7] = (serNumL >> 8); //地址
    buf[8] = (serNumL & 0xFF); //主机命令
    buf[9] = (serNumH >> 8);
    buf[10] = (serNumH & 0xFF);
    mainwindow->diyCMDBuild(buf, 11);
}

