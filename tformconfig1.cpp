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
}

void TFormConfig1::refresh()
{
    ui->chg0->setText(QString::number(static_cast<float>(g_ChgCfgRegs[0] * 1.0 / qPow(10, g_ChgCfgRegsPows[0])), 'f', g_ChgCfgRegsPows[0]));
    ui->chg1->setText(QString::number(static_cast<float>(g_ChgCfgRegs[1] * 1.0 / qPow(10, g_ChgCfgRegsPows[1])), 'f', g_ChgCfgRegsPows[1]));
    ui->chg2->setText(QString::number(static_cast<float>(g_ChgCfgRegs[2] * 1.0 / qPow(10, g_ChgCfgRegsPows[2])), 'f', g_ChgCfgRegsPows[2]));

    ui->dsg0->setText(QString::number(static_cast<float>(g_DsgCfgRegs[0] * 1.0 / qPow(10, g_DsgCfgRegsPows[0])), 'f', g_DsgCfgRegsPows[0]));
    ui->dsg1->setText(QString::number(static_cast<float>(g_DsgCfgRegs[1] * 1.0 / qPow(10, g_DsgCfgRegsPows[1])), 'f', g_DsgCfgRegsPows[1]));
    ui->dsg2->setText(QString::number(static_cast<float>(g_DsgCfgRegs[2] * 1.0 / qPow(10, g_DsgCfgRegsPows[2])), 'f', g_DsgCfgRegsPows[2]));
    ui->dsg3->setText(QString::number(static_cast<float>(g_DsgCfgRegs[3] * 1.0 / qPow(10, g_DsgCfgRegsPows[3])), 'f', g_DsgCfgRegsPows[3]));

    ui->prot0->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[0] * 1.0 / qPow(10, g_ProtectCfgRegsPows[0])), 'f', g_ProtectCfgRegsPows[0]));
    ui->prot1->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[1] * 1.0 / qPow(10, g_ProtectCfgRegsPows[1])), 'f', g_ProtectCfgRegsPows[1]));
    ui->prot2->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[2] * 1.0 / qPow(10, g_ProtectCfgRegsPows[2])), 'f', g_ProtectCfgRegsPows[2]));
    ui->prot3->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[3] * 1.0 / qPow(10, g_ProtectCfgRegsPows[3])), 'f', g_ProtectCfgRegsPows[3]));
    ui->prot4->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[4] * 1.0 / qPow(10, g_ProtectCfgRegsPows[4])), 'f', g_ProtectCfgRegsPows[4]));
    ui->prot5->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[5] * 1.0 / qPow(10, g_ProtectCfgRegsPows[5])), 'f', g_ProtectCfgRegsPows[5]));
    ui->prot6->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[6] * 1.0 / qPow(10, g_ProtectCfgRegsPows[6])), 'f', g_ProtectCfgRegsPows[6]));
    ui->prot7->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[7] * 1.0 / qPow(10, g_ProtectCfgRegsPows[7])), 'f', g_ProtectCfgRegsPows[7]));
    ui->prot8->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[8] * 1.0 / qPow(10, g_ProtectCfgRegsPows[8])), 'f', g_ProtectCfgRegsPows[8]));
    ui->prot9->setText(QString::number(static_cast<float>(g_ProtectCfgRegs[9] * 1.0 / qPow(10, g_ProtectCfgRegsPows[9])), 'f', g_ProtectCfgRegsPows[9]));
}
