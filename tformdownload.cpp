#include "tformdownload.h"
#include "ui_tformdownload.h"
#include "headfile.h"
#include <QTimer>
#include "mainwindow.h"
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>

TFormDownload::TFormDownload(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TFormDownload)
    , timer(new QTimer(this))
{
    ui->setupUi(this);
    //设置窗口标志，确保有边框和标题栏、最小化、关闭、最大化
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint |Qt::WindowMaximizeButtonHint);
    init();
}

TFormDownload::~TFormDownload()
{
    delete ui;
}

void TFormDownload::init()
{
    timer->setInterval(10);
    DownloadFlag = 1;
    pageAddr = 0;
    ui->progressBar->setValue(0);
    ui->master_btn->setEnabled(false);
    DownloadRepeatNum = 0;
    DownloadTime = 0;
    upgradeFlag = 0;
    DownloadTXFlag = 0;
    fileLen = 0;
    fileCRC = 0;
    downloadlen = 0;
    reDownloadFlag = false;
    // 连接定时器超时信号到槽函数
    connect(timer, &QTimer::timeout, this, &TFormDownload::on_timeout);
}

void TFormDownload::endDownload()
{
    DownloadFlag = 1;
    pageAddr = 0;
    ui->pushButton->setEnabled(true);
    timer->stop();
    DownloadTime = 0;
    confirmed = false;
    DownloadTXFlag = 0;
    fileLen = 0;
    fileCRC = 0;
    reDownloadFlag = false;
    downloadlen = 0;
}

void TFormDownload::sendData()
{
    QByteArray buf(600, 0x00);
    if (DownloadTXFlag == 0) {
        return;
    }

    if (DownloadFlag == 2) {  // 握手
        buf[0] = MODULE;
        buf[1] = 0xAA;
        buf[2] = 0x00;
        buf[3] = 0x01;
        mainwindow->diyCMDBuild(buf, 4);
        DownloadTXFlag = 0;
        DownloadTime = 300;
        downloadlen = 0;
        return;
    }

    //擦除
    if(DownloadFlag == 3)
    {
        buf[0] = MODULE;
        buf[1] = 0xAA;
        buf[2] = 0x00;
        buf[3] = 0x02;
        mainwindow->diyCMDBuild(buf, 4);
        DownloadTXFlag = 0;
        DownloadTime = 1000;
        downloadlen = 0;
    }

    //写块
    if(DownloadFlag == 4)
    {
        buf[0] = MODULE;
        buf[1] = 0xAA;
        buf[2] = 0x00;
        buf[3] = 0x03;
        buf[4] = (pageAddr >> 8);
        buf[5] = (pageAddr & 0xFF);
        quint8 copyLen = 0;
        if(fileLen >= (pageAddr + 1) * 128)
        {
            copyLen = 128;
        }else
        {
            copyLen = fileLen - pageAddr * 128;
        }
        buf[6] = (copyLen >> 8);
        buf[7] = (copyLen & 0xFF);
        for(quint8 i = 0; i < copyLen; i++)
        {
            buf[8 + i] = fileBuf[128 * pageAddr + i];
        }
        mainwindow->diyCMDBuild(buf, 8 + copyLen);
        DownloadTXFlag = 0;
        DownloadTime = 300;
        downloadlen = 0;
    }

    //完成
    if(DownloadFlag == 5)
    {
        buf[0] = MODULE;
        buf[1] = 0xAA;
        buf[2] = 0x00;
        buf[3] = 0x04;
        mainwindow->diyCMDBuild(buf, 4);
        DownloadTXFlag = 0;
        DownloadTime = 300;
        downloadlen = 0;
    }
}

void TFormDownload::downloadRespDeal()
{
    quint8 cmd2 = rxBuf[3];
    switch(cmd2)
    {
    case SHAKE_HANDS_CMD:
        if(rxBuf[4] == 6)
        {
            timer->stop();
            DownloadFlag = 3;
            if(pageAddr > 0)
            {
                pageAddr = 0;
                reDownloadFlag = true;
            }
            ui->progressBar->setValue(5);
            ui->plainTextEdit->appendPlainText("握手成功!");
            DownloadTime = 0;
            DownloadTXFlag = 1;
            DownloadRepeatNum = 3;
            timer->start();
        }
        break;
    case ERASURE_CMD:
        if(rxBuf[4] == 6)
        {
            timer->stop();
            DownloadFlag = 4;
            ui->progressBar->setValue(10);
            ui->plainTextEdit->appendPlainText("擦除成功!");
            DownloadTime = 0;
            DownloadTXFlag = 1;
            DownloadRepeatNum = 3;
            timer->start();
        }
        break;
    case WRITE_BLOCK_CMD://数据下载处理
    {
        if(rxBuf[6] == 6)
        {
            pageAddr++;
            int totalPage = (fileLen + 127) / 128;
            int progress = pageAddr * 80 / totalPage + 10;
            if(progress > ui->progressBar->value())
            {
                ui->progressBar->setValue(progress);
            }
            //说明已经下载完成
            if(pageAddr * 128 >= fileLen)
            {
                ui->plainTextEdit->appendPlainText("数据下载完成!");
                DownloadFlag = 5;
            }else
            {
                //继续下载
                DownloadFlag = 4;
            }
            DownloadTime = 0;
            DownloadTXFlag = 1;
            DownloadRepeatNum = 3;
        }else
        {
            DownloadFlag = 4;
        }
        break;
    }
    case FINISH_CMD:
        endDownload();
        if(rxBuf[4] == 6)
        {
            ui->progressBar->setValue(100);

            QMessageBox::information(this, "提示", "升级成功!");
        }else
        {
            ui->plainTextEdit->appendPlainText("升级失败");
            QMessageBox::information(this, "提示", "升级失败!");
        }
        break;
    }
}

QString TFormDownload::getInitDir()
{
    // 使用 QSettings 加载配置文件
    QSettings settings(CONFIG_FILE_PATH, QSettings::IniFormat);
    settings.beginGroup(BASE_CONFIG);
    QString dir = settings.value(DOWNLOAD_FILE_DIR, QDir::currentPath()).toString();
    settings.endGroup();
    return dir;
}

void TFormDownload::updateInitDir(QString dir)
{
    // 使用 QSettings 加载配置文件
    QSettings settings(CONFIG_FILE_PATH, QSettings::IniFormat);
    settings.beginGroup(BASE_CONFIG);
    settings.setValue(DOWNLOAD_FILE_DIR, dir);
    settings.endGroup();
}

void TFormDownload::on_timeout()
{
    if(DownloadFlag < 2)
    {
        return;
    }
    if(DownloadTime > 0)
    {
        DownloadTime--;
        //超时了
        if(DownloadTime == 0)
        {
            //超过重发次数了
            if(DownloadRepeatNum == 0)
            {
                ui->plainTextEdit->appendPlainText("通讯超时!");
                endDownload();
                return;
            }
            DownloadRepeatNum--;
            DownloadTXFlag = 1;
        }
    }
    sendData();
}

void TFormDownload::on_pushButton_clicked()
{
    QString filter = "Parameter(*.bin)|*.bin";
    QString defaultSuffix = "bin";
    QString initialDir = getInitDir();
    QString fileName = QFileDialog::getOpenFileName(this, "请选择升级文件", initialDir, filter);
    if (fileName.isEmpty())
        return;
    ui->lineEdit->setText(fileName);
    updateInitDir(QFileInfo(fileName).absolutePath());
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, "错误", "文件打开失败!");
        ui->master_btn->setEnabled(false);
        return;
    }
    fileLen = file.size();
    if(fileLen > 50000)
    {
        QMessageBox::information(this, "错误", "文件过大!");
        ui->master_btn->setEnabled(false);
        return;
    }
    fileBuf = file.readAll();
    file.close();
    ui->master_btn->setEnabled(true);
    ui->pushButton->setEnabled(false);
}


void TFormDownload::on_master_btn_clicked()
{
    reDownloadFlag = false;
    DownloadFlag = 2;
    //发送标志
    DownloadTXFlag = 1;
    DownloadRepeatNum = 3;
    ui->master_btn->setEnabled(false);
    ui->progressBar->setValue(0);
    timer->setInterval(10);
    timer->start();
}

