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
    quint16 iValue= 0;
    if(lastEditAddr != 2)
    {
        float fValue = ui->lineEdit->text().toFloat();
        iValue= (quint16)(fValue * qPow(10, holdingPow[lastEditAddr]) + 0.5);
    }else
    {
        iValue = ui->lineEdit->text().toInt() * holdingRegs[4] / 100;
    }
    mainwindow->manualWriteOneCMDBuild(lastEditAddr + HOLDING_REG_START, iValue);

}
