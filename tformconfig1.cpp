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
    ui->h1->setText(QString::number(static_cast<float>(holdingRegs[1] * 1.0 / qPow(10, holdingPow[1])), 'f', holdingPow[1]));
    ui->h2->setText(QString::number(holdingRegs[2] * 1.0 / holdingRegs[4] * 100));
    ui->h4->setText(QString::number(static_cast<float>(holdingRegs[4] * 1.0 / qPow(10, holdingPow[4])), 'f', holdingPow[4]));
    ui->h6->setText(QString::number(holdingRegs[6]));
    ui->h7->setText(QString::number(static_cast<float>(holdingRegs[7] * 1.0 / qPow(10, holdingPow[7])), 'f', holdingPow[7]));
    ui->h8->setText(QString::number(static_cast<float>(holdingRegs[8] * 1.0 / qPow(10, holdingPow[8])), 'f', holdingPow[8]));
    ui->h9->setText(QString::number(static_cast<float>(holdingRegs[9] * 1.0 / qPow(10, holdingPow[9])), 'f', holdingPow[9]));
    ui->h10->setText(QString::number(static_cast<float>(holdingRegs[10] * 1.0 / qPow(10, holdingPow[10])), 'f', holdingPow[10]));
    ui->h11->setText(QString::number(static_cast<float>(holdingRegs[11] * 1.0 / qPow(10, holdingPow[11])), 'f', holdingPow[11]));
    ui->h12->setText(QString::number(static_cast<float>(holdingRegs[12] * 1.0 / qPow(10, holdingPow[12])), 'f', holdingPow[12]));
    ui->h13->setText(QString::number(static_cast<float>(holdingRegs[13] * 1.0 / qPow(10, holdingPow[13])), 'f', holdingPow[13]));
    ui->h14->setText(QString::number(static_cast<float>(holdingRegs[14] * 1.0 / qPow(10, holdingPow[14])), 'f', holdingPow[14]));
    ui->h15->setText(QString::number(static_cast<float>(holdingRegs[15] * 1.0 / qPow(10, holdingPow[15])), 'f', holdingPow[15]));
    ui->h16->setText(QString::number(static_cast<float>(holdingRegs[16] * 1.0 / qPow(10, holdingPow[16])), 'f', holdingPow[16]));
    ui->h17->setText(QString::number(holdingRegs[17]));
    ui->c0->blockSignals(true);
    ui->c3->blockSignals(true);
    ui->c5->blockSignals(true);
    ui->c0->setCurrentIndex(holdingRegs[0]);
    ui->c3->setCurrentIndex(holdingRegs[3]);
    ui->c5->setCurrentIndex(holdingRegs[5]);
    preRunModeIndex = ui->c0->currentIndex();
    preDimingModeIndex = ui->c3->currentIndex();
    preFanCtrlModeIndex = ui->c5->currentIndex();
    ui->c0->blockSignals(false);
    ui->c3->blockSignals(false);
    ui->c5->blockSignals(false);
    //恒压限流不能调光
    if(holdingRegs[0] == 0)
    {
        ui->c3->setEnabled(false);
        ui->h2->setEnabled(false);
    }
    //恒流限压可以调光
    if(holdingRegs[0] == 1)
    {
        ui->c3->setEnabled(true);
        //手动模式可以调光
        if(holdingRegs[3] == 5)
        {
            ui->h2->setEnabled(true);
        }else
        {
            ui->h2->setEnabled(false);
        }
    }
    //风扇手动控制可以调功率
    if(holdingRegs[5] == 1)
    {
        ui->h6->setEnabled(true);
    }else
    {
        ui->h6->setEnabled(false);
    }
}

void TFormConfig1::on_c0_currentIndexChanged(int index)
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    ui->c0->blockSignals(true);
    ui->c0->setCurrentIndex(preRunModeIndex);
    ui->c0->blockSignals(false);
    mainwindow->manualWriteOneCMDBuild(HOLDING_REG_START_ADDR, index);
}


void TFormConfig1::on_c3_currentIndexChanged(int index)
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    ui->c3->blockSignals(true);
    ui->c3->setCurrentIndex(preDimingModeIndex);
    ui->c3->blockSignals(false);
    mainwindow->manualWriteOneCMDBuild(HOLDING_REG_START_ADDR + 3, index);
}


void TFormConfig1::on_c5_currentIndexChanged(int index)
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    ui->c5->blockSignals(true);
    ui->c5->setCurrentIndex(preFanCtrlModeIndex);
    ui->c5->blockSignals(false);
    mainwindow->manualWriteOneCMDBuild(HOLDING_REG_START_ADDR + 5, index);
}

