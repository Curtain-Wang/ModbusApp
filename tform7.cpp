#include "tform7.h"
#include "ui_tform7.h"
#include "headfile.h"
#include <QMessageBox>
#include "mainwindow.h"

TForm7::TForm7(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TForm7)
{
    //设置窗口标志，确保有边框和标题栏、最小化、关闭，最大化
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    ui->setupUi(this);
}

TForm7::~TForm7()
{
    delete ui;
}

void TForm7::on_lineEdit_returnPressed()
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    quint16 value = ui->lineEdit->text().toInt();
    quint16 blockStart = ((lastEditAddr & 0xFF00) | 1);
    switch(blockStart)
    {
    case MODBUS_BLOCK_START_CHARGE:
        value = value * qPow(10, g_ChgCfgRegsPows[lastEditAddr - MODBUS_BLOCK_START_CHARGE]);
        break;
    case MODBUS_BLOCK_START_DISCHARGE:
        value = value * qPow(10, g_DsgCfgRegsPows[lastEditAddr - MODBUS_BLOCK_START_DISCHARGE]);
        break;
    case MODBUS_BLOCK_START_PROTECT:
        value = value * qPow(10, g_ProtectCfgRegsPows[lastEditAddr - MODBUS_BLOCK_START_PROTECT]);
        break;
    case MODBUS_BLOCK_START_CTRL:
        value = value * qPow(10, g_SysCtrlgRegsPows[lastEditAddr - MODBUS_BLOCK_START_CTRL]);
        break;
    default:
        return;
    }
    mainwindow->manualWriteOneCMDBuild(lastEditAddr, value);
}
