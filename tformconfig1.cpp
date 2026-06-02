#include "tformconfig1.h"
#include "ui_tformconfig1.h"
#include "mainwindow.h"
#include "headfile.h"
#include <QMessageBox>

TFormConfig1::TFormConfig1(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TFormConfig1)
    , mainwindow(qobject_cast<MainWindow*>(parent))
{
    //设置窗口标志，确保有边框和标题栏、最小化、关闭，最大化
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    ui->setupUi(this);
    init();
}

TFormConfig1::~TFormConfig1()
{
    delete ui;
}

void TFormConfig1::init()
{
    mainwindow->readHoldingRegCMDBuild();
    preRunModeIndex = 0;
    preDimingModeIndex = 0;
    preFanCtrlModeIndex = 0;
    refresh();
}

void TFormConfig1::refresh()
{
    ui->h16385->setText(QString::number(static_cast<float>(g_ChgCfgRegs[0] * 1.0 / qPow(10, g_ChgCfgRegsPows[0])), 'f', g_ChgCfgRegsPows[0]));
    ui->h16386->setText(QString::number(static_cast<float>(static_cast<qint16>(g_ChgCfgRegs[1]) * 1.0 / qPow(10, g_ChgCfgRegsPows[1])), 'f', g_ChgCfgRegsPows[1]));
    ui->h16387->setText(QString::number(static_cast<float>(static_cast<qint16>(g_ChgCfgRegs[2]) * 1.0 / qPow(10, g_ChgCfgRegsPows[2])), 'f', g_ChgCfgRegsPows[2]));

    ui->h16641->setText(QString::number(static_cast<float>(g_DsgCfgRegs[0] * 1.0 / qPow(10, g_DsgCfgRegsPows[0])), 'f', g_DsgCfgRegsPows[0]));
    ui->h16642->setText(QString::number(static_cast<float>(g_DsgCfgRegs[1] * 1.0 / qPow(10, g_DsgCfgRegsPows[1])), 'f', g_DsgCfgRegsPows[1]));
    ui->h16643->setText(QString::number(static_cast<float>(g_DsgCfgRegs[2] * 1.0 / qPow(10, g_DsgCfgRegsPows[2])), 'f', g_DsgCfgRegsPows[2]));
    ui->h16644->setText(QString::number(static_cast<float>(g_DsgCfgRegs[3] * 1.0 / qPow(10, g_DsgCfgRegsPows[3])), 'f', g_DsgCfgRegsPows[3]));

    ui->h16897->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[0] * 1.0 / qPow(10, g_ProtectCfgRegsPows[0])), 'f', g_ProtectCfgRegsPows[0]));
    ui->h16898->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[1] * 1.0 / qPow(10, g_ProtectCfgRegsPows[1])), 'f', g_ProtectCfgRegsPows[1]));
    ui->h16899->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[2] * 1.0 / qPow(10, g_ProtectCfgRegsPows[2])), 'f', g_ProtectCfgRegsPows[2]));
    ui->h16900->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[3] * 1.0 / qPow(10, g_ProtectCfgRegsPows[3])), 'f', g_ProtectCfgRegsPows[3]));
    ui->h16901->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[4] * 1.0 / qPow(10, g_ProtectCfgRegsPows[4])), 'f', g_ProtectCfgRegsPows[4]));
    ui->h16902->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[5] * 1.0 / qPow(10, g_ProtectCfgRegsPows[5])), 'f', g_ProtectCfgRegsPows[5]));
    ui->h16903->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[6] * 1.0 / qPow(10, g_ProtectCfgRegsPows[6])), 'f', g_ProtectCfgRegsPows[6]));
    ui->h16904->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[7] * 1.0 / qPow(10, g_ProtectCfgRegsPows[7])), 'f', g_ProtectCfgRegsPows[7]));
    ui->h16905->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[8] * 1.0 / qPow(10, g_ProtectCfgRegsPows[8])), 'f', g_ProtectCfgRegsPows[8]));
    ui->h16906->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[9] * 1.0 / qPow(10, g_ProtectCfgRegsPows[9])), 'f', g_ProtectCfgRegsPows[9]));

    ui->h17154->setText(QString::number(g_SysCtrlgRegs[1]));
    ui->h17155->setText(QString::number(g_SysCtrlgRegs[2]));
    ui->h17175->setText(QString::number(static_cast<float>(g_SysCtrlgRegs[22] * 1.0 / qPow(10, g_SysCtrlgRegsPows[22])), 'f', g_SysCtrlgRegsPows[22]));
    ui->h17176->setText(QString::number(static_cast<float>(g_SysCtrlgRegs[23] * 1.0 / qPow(10, g_SysCtrlgRegsPows[23])), 'f', g_SysCtrlgRegsPows[23]));
    ui->h17177->setText(QString::number(static_cast<float>(g_SysCtrlgRegs[24] * 1.0 / qPow(10, g_SysCtrlgRegsPows[24])), 'f', g_SysCtrlgRegsPows[24]));
    ui->h17178->setText(QString::number(static_cast<float>(g_SysCtrlgRegs[25] * 1.0 / qPow(10, g_SysCtrlgRegsPows[25])), 'f', g_SysCtrlgRegsPows[25]));
    ui->h17179->setText(QString::number(static_cast<float>(g_SysCtrlgRegs[26] * 1.0 / qPow(10, g_SysCtrlgRegsPows[26])), 'f', g_SysCtrlgRegsPows[26]));
    ui->h17180->setText(QString::number(static_cast<float>(g_SysCtrlgRegs[27] * 1.0 / qPow(10, g_SysCtrlgRegsPows[27])), 'f', g_SysCtrlgRegsPows[27]));


}
