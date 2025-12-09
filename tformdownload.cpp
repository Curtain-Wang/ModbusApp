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
    timer->setInterval(0);
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

    if (DownloadFlag == 2) {  // 下载开启通知
        buf[0] = MODULE;
        if(upgradeFlag == MASTER) //主机升级命令
        {
           buf[1] = 0xF0;
           buf[2] = 0x06;
        }
        for(quint16 i = 3; i < 198; i++)
        {
            buf[i] = 0xaa;
        }
        mainwindow->diyCMDBuild(buf, 198);
        DownloadTXFlag = 0;
        DownloadTime = 300;
        downloadlen = 0;
        return;
    }

    //下载数据
    if(DownloadFlag == 3)
    {
        buf[0] = MODULE;
        if(upgradeFlag == MASTER) //主机升级命令
        {
            buf[1] = 0xF0;
            buf[2] = 0x07;
        }
        buf[3] = pageAddr;
        int copyLen = fileLen >= (pageAddr + 1) * 512 ? 512 : (fileLen - pageAddr * 512);
        memcpy(&buf[4], fileBuf.constData() + (512 * pageAddr), copyLen);
        mainwindow->diyCMDBuild(buf, 516);
        DownloadTXFlag = 0;
        DownloadTime = 300;
        downloadlen = 0;
    }

    //总校验和比对
    if(DownloadFlag == 4)
    {
        buf[0] = MODULE;
        if(upgradeFlag == MASTER) //主机升级命令
        {
            buf[1] = 0xF0;
            buf[2] = 0x08;
        }
        buf[3] = (fileCRC & 0xFF);
        buf[4] = (fileCRC >> 8);
        buf[5] = (swId & 0xFF);
        buf[6] = (swId >> 8);
        buf[7] = (downVer & 0xFF);
        buf[8] = (downVer >> 8);
        mainwindow->diyCMDBuild(buf, 9);
        DownloadTXFlag = 0;
        DownloadTime = 300;
        downloadlen = 0;
    }
}

void TFormDownload::downloadRespDeal()
{
    quint8 cmd2 = rxBuf[2];
    switch(cmd2)
    {
    case UPDATE_CMD://升级命令回复
        timer->stop();
        DownloadFlag = 3;
        if(pageAddr > 0)
        {
            pageAddr = 0;
            reDownloadFlag = true;
        }
        ui->progressBar->setValue(10);
        DownloadTime = 0;
        DownloadTXFlag = 1;
        DownloadRepeatNum = 3;
        timer->start();
        break;
    case DOWNLOAD_DATA_CMD://数据下载处理
    {
        pageAddr++;
        int totalPage = (fileLen + 511) / 512;
        int progress = pageAddr * 80 / totalPage + 10;
        if(progress > ui->progressBar->value())
        {
            ui->progressBar->setValue(progress);
        }
        //说明已经下载完成
        if(pageAddr * 512 >= fileLen)
        {
            ui->plainTextEdit->appendPlainText("数据下载完成, 开始CRC校验");
            DownloadFlag = 4;
        }else
        {
            //继续下载
            DownloadFlag = 3;
        }
        DownloadTime = 0;
        DownloadTXFlag = 1;
        DownloadRepeatNum = 3;
        break;
    }
    case DOWNLOAD_COMPLETE_CHECK_CMD:
        if(rxBuf[3] == 0)
        {
            ui->progressBar->setValue(100);
            ui->plainTextEdit->appendPlainText(QString("已升级到%1版本").arg(downVer, 4, 16, QLatin1Char('0')));
            QMessageBox::information(this, "提示", "升级成功!");
        }else
        {
            ui->plainTextEdit->appendPlainText("升级失败");
            quint16 recCrc;
            memcpy(&recCrc, rxBuf.constData() + 4, sizeof(recCrc));
            quint16 recSwId;
            memcpy(&recSwId, rxBuf.constData() + 6, sizeof(recSwId));
            quint16 ver;
            memcpy(&ver, rxBuf.constData() + 8, sizeof(ver));
            if(recCrc != fileCRC)
            {
                ui->plainTextEdit->appendPlainText("crc校验失败");
            }
            if(swId != recSwId)
            {
                ui->plainTextEdit->appendPlainText("软件标识不匹配");
            }
            if(ver != downVer)
            {
                ui->plainTextEdit->appendPlainText("版本号不匹配");
            }
            QMessageBox::information(this, "提示", "升级失败!");
        }
        DownloadFlag = 5;
        DownloadTime = 0;
        DownloadTXFlag = 1;
        DownloadRepeatNum = 3;
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
                ui->plainTextEdit->appendPlainText("升级失败");
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
    fileCRC = 0;
    for(quint32 i = 0; i < fileLen; i++)
    {
        fileCRC += static_cast<quint8>(fileBuf[i]);
    }
    memcpy(&downVer, fileBuf.constData() + 0x12f8, sizeof(downVer));
    memcpy(&swId, fileBuf.constData() + 0x12fa, sizeof(swId));
    //输出板升级
    if(OUTPUT_SWID == swId)
    {
        ui->plainTextEdit->appendPlainText(QString("当前升级文件为%1，版本号为%2").arg("输出板程序").arg(downVer, 4, 16, QLatin1Char('0')));
    }else if(INPUT_SWID == swId)//输入板升级
    {
        ui->plainTextEdit->appendPlainText(QString("当前升级文件为%1，版本号为%2").arg("输入板程序").arg(downVer, 4, 16, QLatin1Char('0')));
    }else
    {
        QMessageBox::information(this, "错误", "非升级文件!");
        ui->master_btn->setEnabled(false);
        return;
    }
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
    upgradeFlag = MASTER;
    ui->progressBar->setValue(0);
    timer->setInterval(10);
    timer->start();
}

