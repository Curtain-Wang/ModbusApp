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
    float fvalue = ui->lineEdit->text().toFloat();

    if((lastEditAddr == HI_OutVolt || lastEditAddr == HI_OutLimitVolt) && (fvalue < 0 || fvalue > 400))
    {
        QMessageBox::warning(this, tr("警告"), tr("输出电/限压不能超过400V"));
        return;
    }

    if((lastEditAddr == HI_OutCur || lastEditAddr == HI_OutLimitCur || lastEditAddr == HI_FullLightCur) && (fvalue < 0 || fvalue > 30))
    {
        QMessageBox::warning(this, tr("警告"), tr("输出电流、限流、满亮度电流值不能超过30A"));
        return;
    }

    if((lastEditAddr == HI_OutOVPTS) && (fvalue < 0 || fvalue > 420))
    {
        QMessageBox::warning(this, tr("警告"), tr("输出过压保护不能超过420V"));
        return;
    }

    if((lastEditAddr == HI_InOVPTS) && (fvalue < 0 || fvalue > 1000))
    {
        QMessageBox::warning(this, tr("警告"), tr("输入过压保护不能超过1000V"));
        return;
    }

    if((lastEditAddr == HI_OutOCPTS) && (fvalue < 0 || fvalue > 35))
    {
        QMessageBox::warning(this, tr("警告"), tr("输出过流保护不能超过35A"));
        return;
    }

    if((lastEditAddr == HI_InOCPTS) && (fvalue < 0 || fvalue > 40))
    {
        QMessageBox::warning(this, tr("警告"), tr("输入过流保护不能超过40A"));
        return;
    }

    if((lastEditAddr == HI_OTTS) && (fvalue < 0 || fvalue > 160))
    {
        QMessageBox::warning(this, tr("警告"), tr("高温保护不能超过160℃"));
        return;
    }

    quint16 iValue= 0;
    if(lastEditAddr != HI_OutCur)
    {
        float fValue = ui->lineEdit->text().toFloat();
        iValue= (quint16)(fValue * qPow(10, holdingPow[lastEditAddr]) + 0.5);
    }else
    {
        iValue = ui->lineEdit->text().toInt() * holdingRegs[4] / 100;
    }
    mainwindow->manualWriteOneCMDBuild(lastEditAddr + HOLDING_REG_START, iValue);

}
