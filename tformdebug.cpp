#include "tformdebug.h"
#include "ui_tformdebug.h"
#include "mainwindow.h"
#include "headfile.h"
#include <QMessageBox>

TFormDebug::TFormDebug(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TFormDebug)
{
    //设置窗口标志，确保有边框和标题栏、最小化、关闭，最大化
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    ui->setupUi(this);
}

TFormDebug::~TFormDebug()
{
    delete ui;
}

void TFormDebug::init()
{
    mainwindow->readHoldingRegCMDBuild();
    refresh();
}

void TFormDebug::refresh()
{
    ui->h17183->setText(QString::number(static_cast<float>((g_SysCtrlgRegs[30] << 16) + g_SysCtrlgRegs[31]) / 1000000.0, 'f', 6));
    ui->h17185->setText(QString::number(static_cast<float>((g_SysCtrlgRegs[32] << 16) + g_SysCtrlgRegs[33]) / 1000000.0, 'f', 6));
    ui->h17187->setText(QString::number(static_cast<float>((g_SysCtrlgRegs[34] << 16) + g_SysCtrlgRegs[35]) / 1000000.0, 'f', 6));
    ui->h17189->setText(QString::number(static_cast<float>((g_SysCtrlgRegs[36] << 16) + g_SysCtrlgRegs[37]) / 1000000.0, 'f', 6));
    ui->h17191->setText(QString::number(static_cast<float>((g_SysCtrlgRegs[38] << 16) + g_SysCtrlgRegs[39]) / 1000000.0, 'f', 6));
    ui->h17193->setText(QString::number(static_cast<float>((g_SysCtrlgRegs[40] << 16) + g_SysCtrlgRegs[41]) / 1000000.0, 'f', 6));

    ui->h17156->setText(QString::number(static_cast<float>((g_SysCtrlgRegs[3] << 16) + g_SysCtrlgRegs[4]) / 1000000.0, 'f', 6));
    ui->h17158->setText(QString::number(static_cast<float>((g_SysCtrlgRegs[5] << 16) + g_SysCtrlgRegs[6]) / 1000000.0, 'f', 6));
    ui->h17160->setText(QString::number(static_cast<float>((g_SysCtrlgRegs[7] << 16) + g_SysCtrlgRegs[8]) / 1000000.0, 'f', 6));
    ui->h17175->setText(QString::number(static_cast<float>((g_SysCtrlgRegs[22] << 16) + g_SysCtrlgRegs[23]) / 1000000.0, 'f', 6));
    ui->h17177->setText(QString::number(static_cast<float>((g_SysCtrlgRegs[24] << 16) + g_SysCtrlgRegs[25]) / 1000000.0, 'f', 6));

    ui->h17154->setText(QString::number(g_SysCtrlgRegs[1]));
    ui->h17155->setText(QString::number(g_SysCtrlgRegs[2]));
    ui->h17195->setText(QString::number(g_SysCtrlgRegs[42]));
}
