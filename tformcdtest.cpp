#include "tformcdtest.h"
#include "ui_tformcdtest.h"
#include "headfile.h"
#include <QMessageBox>
#include "mainwindow.h"
#include <QTimer>

TFormCDTest::TFormCDTest(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TFormCDTest)
    , timer0(new QTimer(this))
{
    ui->setupUi(this);
    //设置窗口标志，确保有边框和标题栏、最小化、关闭，最大化
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    //发送数据
    connect(timer0, &QTimer::timeout, this, &TFormCDTest::on_timer0_timeout);
    timer0->setInterval(1000);
    count = 0;
    cycleCount = 0;
}

TFormCDTest::~TFormCDTest()
{
    delete ui;
}

void TFormCDTest::on_timer0_timeout()
{
    //充电
    if(count == 0)
    {
        mainwindow->manualWriteOneCMDBuild(0x4301, (1 << 5));
        ui->plainTextEdit->insertPlainText(QString("开始第%1次充电...\n").arg(cycleCount + 1));
    }
    //放电
    if(count == ui->lineEdit->text().toInt())
    {
        mainwindow->manualWriteOneCMDBuild(0x4301, (1 << 3));
        ui->plainTextEdit->insertPlainText(QString("开始第%1次放电...\n").arg(cycleCount + 1));
    }
    count++;
    //重置
    if(count >= ui->lineEdit->text().toInt() + ui->lineEdit_2->text().toInt())
    {
        cycleCount++;
        count = 0;
    }
    //循环结束
    if(cycleCount >= ui->lineEdit_3->text().toInt())
    {
        ui->plainTextEdit->insertPlainText(QString("已循环%1次，退出循环...\n").arg(cycleCount));
        timer0->stop();
        count = 0;
        cycleCount = 0;
    }
}

void TFormCDTest::on_pushButton_clicked()
{
    QString red = "QPushButton { background-color: #EF5350; border: 2px solid #E53935; color: white; font-size: 30px; padding: 10px; border-radius: 10px; width: 100px; height: 50px; text-align: center; } QPushButton:hover { background-color: #e14a47; border: 2px solid #D32F2F; } QPushButton:pressed { background-color: #E53935; border: 2px solid #B71C1C; } QPushButton:checked { background-color: #E53935; border: 2px solid #B71C1C; color: #FFEBEE; box-shadow: 0 0 8px rgba(239, 83, 80, 0.6); font-weight: bold; }";
    QString green = "QPushButton { background-color: #66BB6A; border: 2px solid #43A047; color: white; font-size: 30px; padding: 10px; border-radius: 10px; width: 100px; height: 50px; text-align: center; } QPushButton:hover { background-color: #5AAE5E; border: 2px solid #388E3C; } QPushButton:pressed { background-color: #4CAF50; border: 2px solid #2C6E2E; } QPushButton:checked { background-color: #4CAF50; border: 2px solid #2E7D32; color: #E8F5E9; box-shadow: 0 0 8px rgba(76, 175, 80, 0.6); font-weight: bold; }";


    if(ui->pushButton->text() == "启动")
    {
        timer0->start();
        count = 0;
        cycleCount = 0;
        ui->pushButton->setText("停止");
        ui->pushButton->setStyleSheet(red);
    }else
    {
        timer0->stop();
        count = 0;
        cycleCount = 0;
        ui->pushButton->setText("启动");
        ui->pushButton->setStyleSheet(green);
    }

}

