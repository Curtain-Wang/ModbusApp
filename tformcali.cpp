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
    ui->h17165->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[12]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[12])), 'f', g_SysCtrlgRegsPows[12]));
    ui->h17166->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[13]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[13])), 'f', g_SysCtrlgRegsPows[13]));
    ui->h17167->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[14]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[14])), 'f', g_SysCtrlgRegsPows[14]));
    ui->h17168->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[15]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[15])), 'f', g_SysCtrlgRegsPows[15]));
    ui->h17169->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[16]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[16])), 'f', g_SysCtrlgRegsPows[16]));
    ui->h17170->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[17]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[17])), 'f', g_SysCtrlgRegsPows[17]));
    ui->h17171->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[18]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[18])), 'f', g_SysCtrlgRegsPows[18]));
    ui->h17172->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[19]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[19])), 'f', g_SysCtrlgRegsPows[19]));
    ui->h17173->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[20]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[20])), 'f', g_SysCtrlgRegsPows[20]));
    ui->h17174->setText(QString::number(static_cast<float>(static_cast<qint16>(g_SysCtrlgRegs[21]) * 1.0 / qPow(10, g_SysCtrlgRegsPows[21])), 'f', g_SysCtrlgRegsPows[21]));
}
