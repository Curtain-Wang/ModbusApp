#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QTimer>
#include "headfile.h"
#include "tform1.h"
#include "tformconfig1.h"
#include "tform7.h"
#include "tformdownload.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , connectStatusLabel(new QLabel(this))
    , versionLabel(new QLabel(this))
    , serialPort(new QSerialPort(this))
    , txResetTimer(new QTimer(this))
    , rxResetTimer(new QTimer(this))

{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icon/images/app.ico"));
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshPort()
{
    //清空combox中已经有的串口名
    ui->comboBox_2->clear();
    // 获取系统中所有可用串口
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();

    // 按串口名升序排序
    std::sort(portList.begin(), portList.end(), [](const QSerialPortInfo &a, const QSerialPortInfo &b) {
        return a.portName() < b.portName();
    });

    // 遍历可用串口，将串口名添加到 comboBox中
    for (const QSerialPortInfo &portInfo : portList) {
        ui->comboBox_2->addItem(portInfo.portName());
    }
}

void MainWindow::init()
{
    mainwindow = this;
    refreshPort();
    //发送数据
    sendTimer = new QTimer(this);
    connect(sendTimer, &QTimer::timeout, this, &MainWindow::onSendTimerTimeout);
    sendTimer->setInterval(100);
    sendTimer->start();
    //接收数据
    receiveTimer = new QTimer(this);
    connect(receiveTimer, &QTimer::timeout, this, &MainWindow::onReceiveTimerTimeout);
    receiveTimer->setInterval(10);
    receiveTimer->start();
    //状态栏
    connectStatusLabel->setMinimumWidth(150);
    connectStatusLabel->setStyleSheet("QLabel { background-color : red; color : white; }");
    ui->statusbar->addWidget(connectStatusLabel);
    connectStatusLabel->setText(connStatus.arg("未连接"));

    versionLabel->setMidLineWidth(300);
    versionLabel->setText(versionStr.arg("未知").arg("未知").arg("未知").arg("未知"));
    ui->statusbar->addWidget(versionLabel);
    setWindowTitle(TITLE);
    regPowInit();
    //指示灯定时器相关
    txResetTimer->setSingleShot(true);
    connect(txResetTimer, &QTimer::timeout, this, &MainWindow::on_txResetTimer_timeout);
    rxResetTimer->setSingleShot(true);
    connect(rxResetTimer, &QTimer::timeout, this, &MainWindow::on_rxResetTimer_timeout);

}

void MainWindow::regPowInit()
{
    for(quint8 i = 0; i < 12; i++)
    {
        inputPow[i] = 1;
    }
    inputPow[12] = 0;
    inputPow[13] = 0;

    for(quint8 i = 0; i < REG_NUM; i++)
    {
        holdingPow[i] = 1;
    }
    holdingPow[0] = 0;
    holdingPow[3] = 0;
    holdingPow[5] = 0;
    holdingPow[6] = 0;
    holdingPow[17] = 0;
}

void MainWindow::sendPortData(QByteArray data)
{
    if(data == nullptr)
    {
        sendSerialData(manualSendDataBuf);
    }
    else
    {
        sendSerialData(data);
    }
    //设置等待时间
    waitMessageRemaingTime = 20;
}

void MainWindow::sendSerialData(const QByteArray &data)
{
    //串口未开启
    if(!serialPort->isOpen())
    {
        QMessageBox::critical(this, "错误", "串口未开启!");
        return;
    }
    if(tform1 != nullptr)
    {
        tform1->displayInfo("上位机发送的串口数据：" + data.toHex());
    }
    serialPort->write(data);
    //闪一下绿色
    ui->lab_tx->setStyleSheet(
        "QLabel {"
        "    border-radius: 5px;"
        "    background-color: #00F000;"
        "}"
        );
    if(txResetTimer->isActive())
    {
        txResetTimer->stop();
    }
    txResetTimer->start(500);//500ms后恢复
}

void MainWindow::on_connBtn_2_clicked()
{
    refreshPort();
}

void MainWindow::onSendTimerTimeout()
{
    if(connFlag == UNCONNECTED)
    {
        return;
    }
    if(waitMessageRemaingTime > 0)
    {
        waitMessageRemaingTime--;
    }
    if(dataRefreshRemaingTime > 0)
    {
        dataRefreshRemaingTime--;
    }
    //说明串口空闲，看看有没有手动的命令要下发
    if(waitMessageRemaingTime == 0)
    {
        if(manualFlag == 1)
        {
            //手动命令下发
            sendPortData();
            manualFlag = 0;
        }
        //说明没有手动命令要下发，就判断是否到了刷新时间
        else if(dataRefreshRemaingTime <= 0 && tformDownload == nullptr)
        {
            //获取实时数据
            sendGetRealTimeDataCMD();
        }
    }
}

void MainWindow::sendGetRealTimeDataCMD()
{
    QByteArray buf;
    buf.append(MODULE);
    buf.append(READ_INPUT_CMD);

    //起始地址
    buf.append(static_cast<char>(INPUT_REG_START >> 8));
    buf.append(static_cast<char>(INPUT_REG_START & 0xFF));
    //个数
    buf.append(static_cast<char>(INPUT_REG_NUM >> 8));
    buf.append(static_cast<char>(INPUT_REG_NUM & 0xFF));
    QByteArray crcArray = calculateCRCArray(buf, 6);
    //crC
    buf.append(crcArray[0]);
    buf.append(crcArray[1]);
    sendPortData(buf);
    dataRefreshRemaingTime = DATA_REFRESH_CYCLE;
}

// 计算Modbus-RTU CRC16的方法，返回高低字节的QByteArray
QByteArray MainWindow::calculateCRCArray(const QByteArray &data, int length) {
    uint16_t crc = 0xFFFF; // 初始化CRC为0xFFFF

    for (int i = 0; i < length; i++) {
        crc ^= static_cast<uint8_t>(data[i]); // 将当前字节异或到CRC低位

        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001; // 如果最低位是1，右移后与0xA001异或
            } else {
                crc = crc >> 1; // 否则只右移
            }
        }
    }
    QByteArray crcArray;
    crcArray.append(static_cast<char>(crc & 0xFF));      // CRC低字节
    crcArray.append(static_cast<char>((crc >> 8) & 0xFF)); // CRC高字节
    return crcArray;
}

void MainWindow::on_connBtn_clicked()
{
    if(ui->connBtn->text() == "建立连接")
    {
        if(ui->comboBox_2->currentIndex() == -1)
        {
            QMessageBox::information(this, tr("提示"),
                                     tr("请选择串口!"));
            return;
        }
        serialPort->setBaudRate(ui->cb_br->currentText().toInt());
        serialPort->setPortName(ui->comboBox_2->currentText());
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setParity(QSerialPort::NoParity);
        //连接失败
        if(!serialPort->open(QIODevice::ReadWrite))
        {
            QMessageBox::information(this, tr("错误"),
                                     tr("无法启动串口通讯！！！"));
            connFlag = UNCONNECTED;
            connectStatusLabel->setText(connStatus.arg("未连接"));
            connectStatusLabel->setStyleSheet("QLabel { background-color : red; color : white; }");
            return;
        }
        //连接成功
        else
        {
            connFlag = CONNECTING;
            ui->comboBox_2->setEnabled(false);
            ui->cb_br->setEnabled(false);
            ui->connBtn->setText("断开连接");
            connectStatusLabel->setText(connStatus.arg("连接中..."));
            connectStatusLabel->setStyleSheet("QLabel { background-color : blue; color : white; }");
        }
    }
    else if(ui->connBtn->text() == "断开连接")
    {
        if(!serialPort->isOpen())
        {
            return;
        }
        serialPort->close();
        connFlag = UNCONNECTED;
        ui->comboBox_2->setEnabled(true);
        ui->cb_br->setEnabled(true);
        connectStatusLabel->setText(connStatus.arg("未连接"));
        connectStatusLabel->setStyleSheet("QLabel { background-color : red; color : white; }");
        ui->connBtn->setText("建立连接");

        ui->bms_warn_prot->setText("未连接");
        ui->bms_warn_prot->setProperty("status", "disconnected");
        ui->bms_warn_prot->style()->unpolish(ui->bms_warn_prot);
        ui->bms_warn_prot->style()->polish(ui->bms_warn_prot);
        ui->bms_warn_prot->update();
    }
}

void MainWindow::cacheReceiveData()
{
    if(serialPort->isOpen())
    {
        QByteArray data = serialPort->readAll();
        for (auto byte : data) {
            receiveDataBuf[receiveEndIndex] = byte;
            receiveEndIndex = (receiveEndIndex + 1) % 500;
        }
        if(data.size() > 0)
        {
            //闪一下绿色
            ui->lab_rx->setStyleSheet(
                "QLabel {"
                "    border-radius: 5px;"
                "    background-color: #00F000;"
                "}"
                );
            if(rxResetTimer->isActive())
            {
                rxResetTimer->stop();
            }
            rxResetTimer->start(500);
        }
        //首先更新接收缓冲区的开始坐标
        if(tform1 != nullptr && data.size() > 0)
        {
            tform1->displayInfo("串口未验证消息：" + data.toHex());
        }
    }
}

bool MainWindow::receiveDataCRCCheck(const QByteArray &data)
{
    QByteArray crcResultArray = calculateCRCArray(data, data.length() - 2);
    if(tform1 != nullptr)
    {
        tform1->displayInfo("待校验的下位机数据：" + data.toHex());
    }
    return crcResultArray[0] == data[data.size() - 2] && crcResultArray[1] == data[data.size() - 1];
}

void MainWindow::dealMessage(quint8 *data)
{
    connFlag = CONNECTED;
    connectStatusLabel->setText(connStatus.arg("已连接"));
    connectStatusLabel->setStyleSheet("QLabel { background-color : green; color : white; }");
    //查询命令
    if(data[1] == READ_INPUT_CMD)
    {
        for(quint16 i = 0; i < data[2] / 2; i++)
        {
            inputRegs[i] = ((data[3 + i * 2] << 8) | data[4 + i * 2]);
        }
        refreshInput();
    }
    if(data[1] == READ_HOLDING_CMD)
    {
        for(quint16 i = 0; i < data[2]; i++)
        {
            holdingRegs[i] =((data[3 + i * 2] << 8) | data[4 + i * 2]);
        }
        refreshHolding();
    }
    if(data[1] == WRITE_ONE_CMD)
    {
        readHoldingRegCMDBuild();
    }
    if((data[1] == MASTER_CMD || data[1] == SLAVE_CMD) && data[2] >= UPDATE_CMD && data[2] <= DOWNLOAD_COMPLETE_CHECK_CMD && tformDownload != nullptr)
    {
        tformDownload->downloadRespDeal();
    }
}

void MainWindow::refreshInput()
{
    refresh();
}

void MainWindow::refreshHolding()
{
    tformConfig1->refresh();
}

void MainWindow::refresh()
{
    ui->l0->setText(QString::number(static_cast<float>(inputRegs[0] * 1.0 / qPow(10, inputPow[0])), 'f', inputPow[0]));
    ui->l2->setText(QString::number(static_cast<float>(inputRegs[2] * 1.0 / qPow(10, inputPow[2])), 'f', inputPow[2]));
    ui->l3->setText(QString::number(static_cast<float>(inputRegs[3] * 1.0 / qPow(10, inputPow[3])), 'f', inputPow[3]));
    ui->l5->setText(QString::number(static_cast<float>(inputRegs[5] * 1.0 / qPow(10, inputPow[5])), 'f', inputPow[5]));
    ui->l6->setText(QString::number(static_cast<float>(inputRegs[6] * 1.0 / qPow(10, inputPow[6])), 'f', inputPow[6]));
    ui->l7->setText(QString::number(static_cast<float>(inputRegs[7] * 1.0 / qPow(10, inputPow[7])), 'f', inputPow[7]));
    ui->l8->setText(QString::number(static_cast<float>(inputRegs[8] * 1.0 / qPow(10, inputPow[8])), 'f', inputPow[8]));
    ui->l9->setText(QString::number(static_cast<float>(inputRegs[9] * 1.0 / qPow(10, inputPow[9])), 'f', inputPow[9]));
    ui->l10->setText(QString::number(static_cast<float>(inputRegs[10] * 1.0 / qPow(10, inputPow[10])), 'f', inputPow[10]));
    ui->l11->setText(QString::number(static_cast<float>(inputRegs[11] * 1.0 / qPow(10, inputPow[11])), 'f', inputPow[11]));
    ui->l13->setText(QString::number(inputRegs[13]));
    QString eventStr = getEventText(inputRegs[12]);
    if(eventStr.length() == 0)
    {
        ui->bms_warn_prot->setText(NO_WARN_PROT_STR);
        ui->bms_warn_prot->setProperty("status", "normal");
    }else
    {
        ui->bms_warn_prot->setText(eventStr);
        ui->bms_warn_prot->setProperty("status", "warn");
    }
    ui->bms_warn_prot->style()->unpolish(ui->bms_warn_prot);
    ui->bms_warn_prot->style()->polish(ui->bms_warn_prot);
    ui->bms_warn_prot->update();
    QString version = versionStr.arg((inputRegs[14] >> 8), 2, 16, QLatin1Char('0')).arg((inputRegs[14] & 0xFF), 2, 16, QLatin1Char('0'))
        .arg((inputRegs[15] >> 8), 2, 16, QLatin1Char('0')).arg((inputRegs[15] & 0xFF), 2, 16, QLatin1Char('0'));
    versionLabel->setText(version);
}

QString MainWindow::getEventText(quint16 value)
{
    QString text;
    if((value & 1) == 1)
    {
        text.append(" 输出侧过压保护");
    }
    if(((value >> 1) & 1) == 1)
    {
        text.append(" 输出侧过流保护");
    }
    if(((value >> 2) & 1) == 1)
    {
        text.append(" 输入侧过压保护");
    }
    if(((value >> 3) & 1) == 1)
    {
        text.append(" 输入侧过流保护");
    }
    if(((value >> 4) & 1) == 1)
    {
        text.append(" 高温保护");
    }
    if(((value >> 5) & 1) == 1)
    {
        text.append(" 湿度过高保护");
    }
    return text;
}

void MainWindow::readHoldingRegCMDBuild()
{
    if(manualFlag == 1)
    {
        QMessageBox::information(this, "冲突", "当前有其他手动命令在发送, 请稍后再试!");
        return;
    }
    manualSendDataBuf.clear();
    manualSendDataBuf.append(MODULE);
    manualSendDataBuf.append(READ_HOLDING_CMD);
    manualSendDataBuf.append(static_cast<char>(HOLDING_REG_START_ADDR >> 8));
    manualSendDataBuf.append(static_cast<char>(HOLDING_REG_START_ADDR & 0xFF));
    manualSendDataBuf.append(static_cast<char>(HOLDING_REG_NUM >> 8));
    manualSendDataBuf.append(static_cast<char>(HOLDING_REG_NUM & 0xFF));
    QByteArray crcArray = calculateCRCArray(manualSendDataBuf, 6);
    manualSendDataBuf.append(crcArray[0]);
    manualSendDataBuf.append(crcArray[1]);
    manualFlag = 1;

}

void MainWindow::manualWriteOneCMDBuild(quint16 addr, quint16 value)
{
    if(manualFlag == 1)
    {
        QMessageBox::information(this, "冲突", "当前有其他手动命令在发送, 请稍后再试!");
        return;
    }
    manualSendDataBuf.clear();
    manualSendDataBuf.append(MODULE);
    manualSendDataBuf.append(WRITE_ONE_CMD);
    manualSendDataBuf.append(addr >> 8);
    manualSendDataBuf.append(addr & 0xFF);
    manualSendDataBuf.append(value >> 8);
    manualSendDataBuf.append(value & 0xFF);
    QByteArray crcArray = calculateCRCArray(manualSendDataBuf, 6);
    manualSendDataBuf.append(crcArray[0]);
    manualSendDataBuf.append(crcArray[1]);
    manualFlag = 1;
}

void MainWindow::diyCMDBuild(QByteArray data, quint16 len)
{
    if(manualFlag == 1)
    {
        QMessageBox::information(this, "冲突", "当前有其他手动命令在发送, 请稍后再试!");
        return;
    }
    manualSendDataBuf.clear();
    for(quint16 i = 0; i < len; i++)
    {
        manualSendDataBuf.append(data[i]);
    }
    QByteArray crcArray = calculateCRCArray(manualSendDataBuf, len);
    manualSendDataBuf.append(crcArray[0]);
    manualSendDataBuf.append(crcArray[1]);
    manualFlag = 1;
}

quint16 MainWindow::getMessageSize()
{
    int cmd = static_cast<uint8_t>(receiveDataBuf[(receiveStartIndex + 1) % 500]);
    if(cmd == 3 || cmd == 4)
    {
        return receiveDataBuf[(receiveStartIndex + 2) % 500] + 5;
    }
    if(cmd == 6)
    {
        return 8;
    }
    if(cmd == 16)
    {
        return 16;
    }
    if(cmd == 0xF0 || cmd == 0xE0)
    {
        quint8 cmd2 = static_cast<uint8_t>(receiveDataBuf[(receiveStartIndex + 2) % 500]);
        switch(cmd2)
        {
        case 0x06:
            return 5;
        case 0x07:
            return 6;
        case 0x08:
            return 12;
        }
    }
    return 0;
}

void MainWindow::onReceiveTimerTimeout()
{
    if(connFlag == UNCONNECTED)
    {
        return;
    }
    cacheReceiveData();
    //当缓冲区的消息长度大于messageSize，那说明可能存在一条完整的响应
    while (receiveEndIndex != receiveStartIndex) {
        int module = static_cast<uint8_t>(receiveDataBuf[receiveStartIndex]);
        int cmd = static_cast<uint8_t>(receiveDataBuf[(receiveStartIndex + 1) % 500]);
        //没有匹配到开始
        if(module != MODULE || (cmd != 3 && cmd != 6 && cmd != 0x10 && cmd != 4 && cmd != 0xF0 && cmd != 0xE0))
        {
            //更新开始点
            receiveStartIndex = (receiveStartIndex + 1) % 500;
            continue;
        }
        //匹配到开始,再匹配下长度是否符合
        int messageSize = getMessageSize();
        if((receiveEndIndex + 500 - receiveStartIndex) % 500 < messageSize){
            //消息还没接收完整，等下一次定时去接,不更新开始点
            break;
        }
        if(messageSize == 0)
        {
            //更新开始点
            receiveStartIndex = (receiveStartIndex + 1) % 500;
            continue;
        }
        rxBuf.clear();
        //构建消息
        for (int var = 0; var < messageSize; var++) {
            rxBuf.append(receiveDataBuf[(receiveStartIndex + var) % 500]);
        }
        //判断是否是一个完整的消息
        if(receiveDataCRCCheck(rxBuf))
        {
            //首先更新接收缓冲区的开始坐标
            if(tform1 != nullptr)
            {
                tform1->displayInfo("串口上传上来且验证通过的一条消息：" + rxBuf.toHex());
            }
            receiveStartIndex = (receiveStartIndex + messageSize) % 500;
            //清空等待时间
            waitMessageRemaingTime = 0;
            dealMessage(reinterpret_cast<quint8*>(rxBuf.data()));
            break;
        }
        //crc校验失败
        else
        {
            //更新开始点
            receiveStartIndex = (receiveStartIndex + 1) % 500;
            continue;
        }
    }
}



void MainWindow::onTFormDestroyed(QObject *obj)
{
    if(obj == tform1)
    {
        tform1 = nullptr;
    }
    if(obj == tform7)
    {
        tform7 = nullptr;
    }
    if(obj == tformConfig1)
    {
        tformConfig1 = nullptr;
    }
    if(obj == tformDownload)
    {
        tformDownload = nullptr;
    }
}


void MainWindow::on_pushButton_4_clicked()
{
    if(tformConfig1 == nullptr)
    {
        tformConfig1 = new TFormConfig1(this);
        tformConfig1->setAttribute(Qt::WA_DeleteOnClose);
        connect(tformConfig1, &TFormConfig1::destroyed, this, &MainWindow::onTFormDestroyed);
    }
    tformConfig1->show();
}


void MainWindow::on_pushButton_8_clicked()
{
    if(tformDownload == nullptr)
    {
        tformDownload = new TFormDownload(this);
        tformDownload->setAttribute(Qt::WA_DeleteOnClose);
        connect(tformDownload, &TFormDownload::destroyed, this, &MainWindow::onTFormDestroyed);
    }
    tformDownload->show();
}


void MainWindow::on_pushButton_6_clicked()
{
    if(tform1 == nullptr)
    {
        tform1 = new TForm1(this);
        tform1->setAttribute(Qt::WA_DeleteOnClose);
        connect(tform1, &TForm1::destroyed, this, &MainWindow::onTFormDestroyed);
    }
    tform1->show();
}

void MainWindow::on_txResetTimer_timeout()
{
    //恢复灰色
    ui->lab_tx->setStyleSheet(
        "QLabel {"
        "    border-radius: 5px;"
        "    background-color: #D3D3D3;"
        "}"
        );
}

void MainWindow::on_rxResetTimer_timeout()
{
    //恢复灰色
    ui->lab_rx->setStyleSheet(
        "QLabel {"
        "    border-radius: 5px;"
        "    background-color: #D3D3D3;"
        "}"
        );
}

