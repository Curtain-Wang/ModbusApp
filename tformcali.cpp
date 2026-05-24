#include "tformcali.h"
#include "ui_tformcali.h"
#include "mainwindow.h"
#include "headfile.h"
#include <QMessageBox>

TFormCali::TFormCali(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TFormCali)
    , mainwindow(qobject_cast<MainWindow*>(parent))
{
    //设置窗口标志，确保有边框和标题栏、最小化、关闭，最大化
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    ui->setupUi(this);
}

TFormCali::~TFormCali()
{
    delete ui;
}

void TFormCali::refresh()
{
    ui->sys12->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[12]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[12])), 'f', g_SysCtrlgRegsPows[12]));
    ui->sys13->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[13]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[13])), 'f', g_SysCtrlgRegsPows[13]));
    ui->sys14->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[14]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[14])), 'f', g_SysCtrlgRegsPows[14]));
    ui->sys15->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[15]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[15])), 'f', g_SysCtrlgRegsPows[15]));
    ui->sys16->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[16]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[16])), 'f', g_SysCtrlgRegsPows[16]));
    ui->sys17->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[17]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[17])), 'f', g_SysCtrlgRegsPows[17]));
    ui->sys18->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[18]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[18])), 'f', g_SysCtrlgRegsPows[18]));
    ui->sys19->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[19]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[19])), 'f', g_SysCtrlgRegsPows[19]));
    ui->sys20->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[20]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[20])), 'f', g_SysCtrlgRegsPows[20]));
    ui->sys21->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[21]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[21])), 'f', g_SysCtrlgRegsPows[21]));
}

void TFormCali::on_pushButton_2_clicked()
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    mainwindow->manualWriteOneCMDBuild(0x4304, 1);
}


void TFormCali::on_pushButton_3_clicked()
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    mainwindow->manualWriteOneCMDBuild(0x4304, 2);
}


void TFormCali::on_pushButton_4_clicked()
{
    // 获取电压校准值并转换为短整数
    qint16 tt = ui->lineEdit_4->text().toDouble() * 100;
    mainwindow->manualWriteOneCMDBuild(0x4305, tt);
}


void TFormCali::on_pushButton_5_clicked()
{
    qint16 tt = ui->lineEdit_5->text().toDouble() * 100;
    mainwindow->manualWriteOneCMDBuild(0x4306, tt);
}


void TFormCali::on_pushButton_6_clicked()
{
    qint16 tt = ui->lineEdit_6->text().toDouble() * 100;
    mainwindow->manualWriteOneCMDBuild(0x4307, tt);
}


void TFormCali::on_pushButton_7_clicked()
{
    qint16 tt = ui->lineEdit_7->text().toDouble() * 100;
    mainwindow->manualWriteOneCMDBuild(0x4308, tt);
}


void TFormCali::on_pushButton_8_clicked()
{
    qint16 tt = ui->lineEdit_8->text().toDouble() * 10;
    mainwindow->manualWriteOneCMDBuild(0x4309, tt);
}


void TFormCali::on_pushButton_9_clicked()
{
    qint16 tt = ui->lineEdit_9->text().toDouble() * 10;
    mainwindow->manualWriteOneCMDBuild(0x430A, tt);
}


void TFormCali::on_pushButton_10_clicked()
{
    qint16 tt = ui->lineEdit_10->text().toDouble() * 10;
    mainwindow->manualWriteOneCMDBuild(0x430B, tt);
}


void TFormCali::on_pushButton_11_clicked()
{
    qint16 tt = ui->lineEdit_11->text().toDouble() * 10;
    mainwindow->manualWriteOneCMDBuild(0x430C, tt);
}

