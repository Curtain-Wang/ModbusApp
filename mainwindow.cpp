#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QTimer>
#include <QKeyEvent>
#include "headfile.h"
#include "tform1.h"
#include "tformconfig1.h"
#include "tform7.h"
#include "tformdownload.h"
#include <QFile>
#include <QSettings>
#include <QDir>
#include "tformdatarecord.h"
#include "tformsernum.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , connectStatusLabel(new QLabel(this))
    , versionLabel(new QLabel(this))
    , runTimeLabel(new QLabel(this))
    , serialPort(new QSerialPort(this))
    , txResetTimer(new QTimer(this))
    , rxResetTimer(new QTimer(this))
    , saveDataTimer(new QTimer(this))
    , reconnectTimer(new QTimer(this))

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
    initConfigFile();
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
    //版本号
    versionLabel->setMidLineWidth(300);
    versionLabel->setText(versionStr.arg("未知").arg("未知").arg("未知").arg("未知"));
    ui->statusbar->addWidget(versionLabel);
    //持续运行时间
    runTimeLabel->setMidLineWidth(800);
    runTimeLabel->setText(runTimeStr.arg(lastRunSecond / 3600).arg(lastRunSecond % 3600 / 60).arg(lastRunSecond % 60));
    // 设置标签右对齐
    runTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->statusbar->addPermanentWidget(runTimeLabel, 1);  // 第二个参数是伸缩因子
    setWindowTitle(QString(TITLE).arg(batSerNum));
    regPowInit();
    //指示灯定时器相关
    txResetTimer->setSingleShot(true);
    connect(txResetTimer, &QTimer::timeout, this, &MainWindow::on_txResetTimer_timeout);
    rxResetTimer->setSingleShot(true);
    connect(rxResetTimer, &QTimer::timeout, this, &MainWindow::on_rxResetTimer_timeout);
    // 确保控件可以获得焦点
    setFocusPolicy(Qt::StrongFocus);
    // 并且实际获得了焦点
    setFocus();
    preRunModeIndex = ui->rbtn0->isChecked() ? 0 : 1;
    connect(saveDataTimer, &QTimer::timeout, this, &MainWindow::on_saveDataTimer_timeout);
    connect(reconnectTimer, &QTimer::timeout, this, &MainWindow::on_reconnectTimer_timeout);
    reconnectTimer->setInterval(5000);
    menuBar()->setStyleSheet("QMenu { background-color: white; }"
                             "QMenu::item { color: black; padding: 5px 20px; }"
                             "QMenu::item:selected {color: black; border: 1px solid gray;}"
                             "QMenu::item:hover {color: black; border: 1px solid gray;}");
}

void MainWindow::initConfigFile()
{
    // 检查配置文件是否存在
    if (!QFile::exists(CONFIG_FILE_PATH)) {
        QString configFilePath = CONFIG_FILE_PATH;
        QSettings settings(configFilePath, QSettings::IniFormat);

        // 设置默认值
        settings.beginGroup(DATA_RECORD_CONFIG);
        settings.setValue(DATA_RECORD_FILE_PATH, QDir::currentPath());
        settings.setValue(DATA_RECORD_CYCLE, DEFAULT_DATA_RECORD_CYCLE);
        settings.endGroup();
        settings.beginGroup(BASE_CONFIG);
        settings.setValue(CONTINUOUS_RUN_TIME, 0);
        settings.endGroup();
    }

    // 使用 QSettings 加载配置文件
    QSettings settings(CONFIG_FILE_PATH, QSettings::IniFormat);
    // 读取 DATA_RECORD_CONFIG 组中的参数
    settings.beginGroup(DATA_RECORD_CONFIG);
    dataRecordFilePath = settings.value(DATA_RECORD_FILE_PATH, QDir::currentPath()).toString();
    dataRecordCycle = settings.value(DATA_RECORD_CYCLE, DEFAULT_DATA_RECORD_CYCLE).toInt();
    settings.endGroup();
    settings.beginGroup(BASE_CONFIG);
    lastRunSecond = settings.value(CONTINUOUS_RUN_TIME, 0).toInt();
    saveDataTimer->setInterval(dataRecordCycle * 1000);
}

void MainWindow::regPowInit()
{
    for(quint8 i = 0; i < 12; i++)
    {
        inputPow[i] = 1;
    }
    inputPow[12] = 0;
    inputPow[13] = 0;
    inputPow[17] = 1;

    for(quint8 i = 0; i < REG_NUM; i++)
    {
        holdingPow[i] = 1;
    }
    holdingPow[0] = 0;
    holdingPow[3] = 0;
    holdingPow[5] = 0;
    holdingPow[6] = 0;
    holdingPow[17] = 0;
    holdingPow[18] = 0;
    holdingPow[19] = 0;
    holdingPow[20] = 0;
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
    runTimeDeal();
    if(connFlag == UNCONNECTED)
    {
        return;
    }
    if(waitMessageRemaingTime > 0)
    {
        waitMessageRemaingTime--;
        //连续超时大于三次未回复就断开连接
        if(waitMessageRemaingTime == 0)
        {
            timeoutTimes++;
            if(timeoutTimes > 3)
            {
                timeoutTimes = 0;
                disconnect();
                //开启重连
                if(reconnectFlag == 0)
                {
                    startReonnect();
                }
            }
        }
        return;
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
            ui->connBtn->setStyleSheet(RED_BUTTON_STYLE);
            sendTimer->start();
            receiveTimer->start();
            connectStatusLabel->setText(connStatus.arg("连接中..."));
            connectStatusLabel->setStyleSheet("QLabel { background-color : blue; color : white; }");
            portName = ui->comboBox_2->currentText();
            baudRate = ui->cb_br->currentText().toInt();
        }
    }
    else if(ui->connBtn->text() == "断开连接")
    {
        disconnect();
        stopReconnect();
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
    timeoutTimes = 0;
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
        quint16 addr = ((data[2] << 8) | data[3]);
        //启动失败
        if(addr == HI_OPEN + HOLDING_REG_START && data[4] == 0xFF)
        {
            QMessageBox::warning(this, "告警", "检测到当前输出端已有电压, 不能手动启动!");
        }else
        {
            holdingRegs[addr - HOLDING_REG_START] = ((data[4] << 8) | data[5]);
        }
        refreshHolding();
    }
    if(data[1] == MASTER_CMD)
    {
        if(data[2] >= UPDATE_CMD && data[2] <= DOWNLOAD_COMPLETE_CHECK_CMD && tformDownload != nullptr)
        {
            tformDownload->downloadRespDeal();
        }
        if(data[2] == SERIAL_NUM_CMD)
        {
            QMessageBox::warning(this, "提示", "写入成功!");
        }
    }
}

void MainWindow::refreshInput()
{
    refresh();
}

void MainWindow::refreshHolding()
{
    if(tformConfig1 != nullptr)
        tformConfig1->refresh();
}

void MainWindow::refresh()
{
    ui->l0->setText(QString::number(static_cast<float>(inputRegs[0] * 1.0 / qPow(10, inputPow[0])), 'f', inputPow[0]));
    ui->l17->setText(QString::number(static_cast<float>(inputRegs[17] * 1.0 / qPow(10, inputPow[17])), 'f', inputPow[17]));
    ui->l3->setText(QString::number(static_cast<float>(inputRegs[3] * 1.0 / qPow(10, inputPow[3])), 'f', inputPow[3]));
    ui->l5->setText(QString::number(static_cast<float>(inputRegs[5] * 1.0 / qPow(10, inputPow[5])), 'f', inputPow[5]));
    ui->l6->setText(QString::number(static_cast<float>(inputRegs[6] * 1.0 / qPow(10, inputPow[6])), 'f', inputPow[6]));
    ui->l7->setText(QString::number(static_cast<float>(inputRegs[7] * 1.0 / qPow(10, inputPow[7])), 'f', inputPow[7]));
    ui->l8->setText(QString::number(static_cast<float>(inputRegs[8] * 1.0 / qPow(10, inputPow[8])), 'f', inputPow[8]));
    ui->l9->setText(QString::number(static_cast<float>(inputRegs[9] * 1.0 / qPow(10, inputPow[9])), 'f', inputPow[9]));
    ui->l2->setText(QString::number(static_cast<float>(inputRegs[2] * 1.0 / qPow(10, inputPow[2])), 'f', inputPow[2]));
    ui->l18->setText(QString::number(inputRegs[18]));
    ui->l13->setText(QString::number(inputRegs[13]));
    ui->lPow->setText(QString::number(inputRegs[3] * inputRegs[5] / 100));
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
    QString version = versionStr.arg((inputRegs[14] >> 8), 2, 16, QLatin1Char('0')).arg((inputRegs[14] & 0xFF), 2, 16, QLatin1Char('0'));
    versionLabel->setText(version);
    //启停
    if(inputRegs[1] == 1)
    {
        ui->pushButton_6->setText("停止");
        ui->pushButton_6->setStyleSheet(RED_BUTTON_STYLE);
        if(!saveDataTimer->isActive())
        {
            saveDataTimer->start();
        }
    }else
    {
        ui->pushButton_6->setText("启动");
        ui->pushButton_6->setStyleSheet(GREEN_BUTTON_STYLE);
        if(saveDataTimer->isActive())
        {
            saveDataTimer->stop();
        }
    }
    //运行模式
    if(inputRegs[19] == 0)
    {
        ui->rbtn0->setChecked(true);
        preRunModeIndex = 0;
    }else
    {
        ui->rbtn1->setChecked(true);
        preRunModeIndex = 1;
    }
    batSerNum.clear();
    for(quint8 i = 20; i < 30; i++)
    {
        if(inputRegs[i] == -1)
        {
            inputRegs[i] = 0;
        }
        QString strVal = QString("%1%2").arg(QChar(inputRegs[i] & 0xFF)).arg(QChar(inputRegs[i] >> 8));
        batSerNum += strVal;
    }
    setWindowTitle(QString(TITLE).arg(batSerNum));
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
    if(((value >> 7) & 1) == 1)
    {
        text.append(" 参数配置故障");
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
    if(cmd == READ_HOLDING_CMD || cmd == READ_INPUT_CMD)
    {
        return receiveDataBuf[(receiveStartIndex + 2) % 500] + 5;
    }
    if(cmd == WRITE_ONE_CMD)
    {
        return 8;
    }
    if(cmd == MASTER_CMD)
    {
        quint8 cmd2 = static_cast<uint8_t>(receiveDataBuf[(receiveStartIndex + 2) % 500]);
        switch(cmd2)
        {
        case UPDATE_CMD:
            return 5;
        case DOWNLOAD_DATA_CMD:
            return 6;
        case DOWNLOAD_COMPLETE_CHECK_CMD:
            return 12;
        case SERIAL_NUM_CMD:
            return 25;
        }
    }
    return 0;
}

void MainWindow::updateSaveDataInterval(int second)
{
    saveDataTimer->setInterval(second * 1000);
}

void MainWindow::startReonnect()
{
    //刷新端口
    refreshPort();
    //开启定时重连
    reconnectTimer->start();
    reconnectFlag = 1;
}

void MainWindow::stopReconnect()
{
    //停止重连
    if(reconnectTimer->isActive())
    {
        reconnectTimer->stop();
    }
    reconnectFlag = 0;
}

void MainWindow::disconnect()
{
    receiveTimer->stop();
    if(serialPort->isOpen())
    {
        serialPort->close();
    }
    connFlag = UNCONNECTED;
    ui->comboBox_2->setEnabled(true);
    ui->cb_br->setEnabled(true);
    connectStatusLabel->setText(connStatus.arg("未连接"));
    connectStatusLabel->setStyleSheet("QLabel { background-color : red; color : white; }");
    ui->connBtn->setText("建立连接");
    ui->connBtn->setStyleSheet(GREEN_BUTTON_STYLE);
    ui->bms_warn_prot->setText("未连接");
    ui->bms_warn_prot->setProperty("status", "disconnected");
    ui->bms_warn_prot->style()->unpolish(ui->bms_warn_prot);
    ui->bms_warn_prot->style()->polish(ui->bms_warn_prot);
    ui->bms_warn_prot->update();
}

bool MainWindow::ensureSaveDirectoryExists(QString serialNumberStr)
{
    QDir dir(QCoreApplication::applicationDirPath() + "/Save/" + serialNumberStr);
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}

void MainWindow::initializeCSVFile(QTextStream &out)
{
    QStringList headers;
    headers.append("时间");
    headers.append("运行模式");
    headers.append("告警保护事件");
    headers.append("输入电压外侧(V)");
    headers.append("输入电压内侧(V)");
    headers.append("谐振腔电流(V)");
    headers.append("输出电压(V)");
    headers.append("设定输出电压(V)(恒压模式有效)");
    headers.append("输出电流(A)");
    headers.append("设定输出电流(A)(恒流模式有效)");
    headers.append("输出功率(W)");
    headers.append("底部散热器温度(℃)");
    headers.append("电感温度(℃)");
    headers.append("变压器温度(℃)");
    headers.append("内腔温度(℃)");
    headers.append("电压环数控值");
    headers.append("电流环数控值");
    out << headers.join("\t") << "\n";
}

void MainWindow::writeDataToCSV(QTextStream &out, const QDateTime &currentTime)
{
    QStringList data;
    data << currentTime.toString("yyyy-MM-dd hh:mm:ss");
    if(inputRegs[19] == 0)
        data << "恒压模式";
    else if(inputRegs[19] == 1)
        data << "恒流模式";
    else
        data << "手动模式";
    //告警/保护
    QString text = getEventText(inputRegs[12]);
    if(text.isEmpty())
    {
        data << "无事件";
    }else
    {
        data << getEventText(inputRegs[12]);
    }
    //输入电压外侧
    data << QString::number(inputRegs[17] / 10, 'f', 1);
    //输入电压内侧
    data << QString::number(inputRegs[0] / 10, 'f', 1);
    //谐振腔电流
    data << QString::number(inputRegs[2] / 10, 'f', 1);
    //输出电压
    data << QString::number(inputRegs[3] / 10, 'f', 1);
    //设定输出电压
    data << QString::number(holdingRegs[1] / 10, 'f', 1);
    //输出电流
    data << QString::number(inputRegs[5] / 10, 'f', 1);
    //设定输出电流
    data << QString::number(holdingRegs[2] / 10, 'f', 1);
    //输出功率
    data << QString::number(inputRegs[3] * inputRegs[5] / 100);
    //底部散热器温度
    data << QString::number(inputRegs[6] / 10, 'f', 1);
    //电感温度
    data << QString::number(inputRegs[7] / 10, 'f', 1);
    //变压器温度
    data << QString::number(inputRegs[8] / 10, 'f', 1);
    //内腔温度
    data << QString::number(inputRegs[9] / 10, 'f', 1);
    //电压环数控值
    data << QString::number(inputRegs[13]);
    //电流环数控值
    data << QString::number(inputRegs[18]);
    out << data.join("\t") << "\n";
}

void MainWindow::runTimeDeal()
{
    //未连接、未启动、有事件
    if(connFlag == UNCONNECTED || inputRegs[1] == 0 || inputRegs[12] > 0)
    {
        if(runSecond != 0)
        {
            lastRunSecond = runSecond / 10;
            QSettings settings(CONFIG_FILE_PATH, QSettings::IniFormat);
            settings.beginGroup(BASE_CONFIG);
            settings.setValue(CONTINUOUS_RUN_TIME, lastRunSecond);
            settings.endGroup();
        }
        runSecond = 0;
    }else
    {
        runSecond++;
        runTimeLabel->setText(QString(runTimeStr).arg(runSecond / 36000).arg(runSecond % 36000 / 600).arg(runSecond % 600 / 10));
    }
}

void MainWindow::onReceiveTimerTimeout()
{
    if(connFlag == UNCONNECTED)
    {
        return;
    }
    cacheReceiveData();
    //当缓冲区的消息长度大于messageSize，那说明可能存在一条完整的响应
    while ((receiveEndIndex + 500 - receiveStartIndex) % 500 >= 4) {
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
    if(obj == tformDataRecord)
    {
        tformDataRecord = nullptr;
    }
    if(obj == tformSerNum)
    {
        tformSerNum = nullptr;
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
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    if(ui->pushButton_6->text() == "启动")
    {
        mainwindow->manualWriteOneCMDBuild(HI_OPEN + HOLDING_REG_START, 1);
        runSecond = 0;
    }else
    {
        mainwindow->manualWriteOneCMDBuild(HI_OPEN + HOLDING_REG_START, 0);
    }
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

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_T) {
        if(tform1 == nullptr)
        {
            tform1 = new TForm1(this);
            tform1->setAttribute(Qt::WA_DeleteOnClose);
            connect(tform1, &TForm1::destroyed, this, &MainWindow::onTFormDestroyed);
        }
        tform1->show();
    } else {
        // 调用父类的实现处理其他按键
        QWidget::keyPressEvent(event);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn;
    resBtn = QMessageBox::question(this, "提示",
                                   "确定要退出嘛？",
                                   QMessageBox::No | QMessageBox::Yes,
                                   QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::on_rbtn0_clicked(bool checked)
{
    if(preRunModeIndex == 0)
    {
        return;
    }
    ui->rbtn1->blockSignals(true);
    ui->rbtn1->setChecked(true);
    ui->rbtn1->blockSignals(false);
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    manualWriteOneCMDBuild(HOLDING_REG_START_ADDR, 0);
}

void MainWindow::on_saveDataTimer_timeout()
{
    //只记录启动状态下的数据
    if(connFlag == CONNECTED && inputRegs[1] == 1)
    {
        QDateTime currentTime = QDateTime::currentDateTime();
        QString serialNumberStr = batSerNum;
        QString fileName = QString("%1-%2-%3-%4")
                               .arg(currentTime.date().year())
                               .arg(currentTime.date().month(), 2, 10, QChar('0'))
                               .arg(currentTime.date().day(), 2, 10, QChar('0'))
                               .arg(serialNumberStr);
        QString filePath = QString("%1/Save/%2")
                               .arg(dataRecordFilePath)
                               .arg(serialNumberStr);
        QDir dir(filePath);
        if (!dir.exists()) {
            if(tform1 != nullptr)
            {
                tform1->displayInfo("未找到路径，已创建");
            }
            dir.mkpath(".");
        }
        filePath = QString("%1/Save/%2/%3.xls")
                       .arg(dataRecordFilePath)
                       .arg(serialNumberStr)
                       .arg(fileName);
        if(!ensureSaveDirectoryExists(serialNumberStr))
        {
            if(tform1 != nullptr)
            {
                tform1->displayInfo("无法创建保存目录");
            }
            return;
        }
        if(filePath != csvFile.fileName() || !csvFile.isOpen())
        {
            csvFile.setFileName(filePath);
            if (!csvFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                if(tform1 != nullptr)
                {
                    tform1->displayInfo("无法打开文件");
                }
                return;
            }
        }
        QTextStream out(&csvFile);

        if (csvFile.size() == 0) {
            initializeCSVFile(out);
        }
        writeDataToCSV(out, currentTime);
        csvFile.flush();

    }
}

void MainWindow::on_rbtn1_clicked(bool checked)
{
    if(preRunModeIndex == 1)
    {
        return;
    }
    ui->rbtn0->blockSignals(true);
    ui->rbtn0->setChecked(true);
    ui->rbtn0->blockSignals(false);
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    manualWriteOneCMDBuild(HOLDING_REG_START_ADDR, 1);
}


void MainWindow::on_actionRefreshPort_triggered()
{
    refreshPort();
}


void MainWindow::on_actDataRecord_triggered()
{
    if(tformDataRecord == nullptr)
    {
        tformDataRecord = new TFormDataRecord(this);
        tformDataRecord->setAttribute(Qt::WA_DeleteOnClose);
        connect(tformDataRecord, &TFormDataRecord::destroyed, this, &MainWindow::onTFormDestroyed);
    }
    tformDataRecord->show();
}

void MainWindow::on_reconnectTimer_timeout()
{
    //确保是建立连接
    if(connFlag == UNCONNECTED)
    {
        refreshPort();
        QString searchText = portName;  // 要查找的字符串
        int index = ui->comboBox_2->findText(searchText);
        if (index != -1) {
            ui->comboBox_2->setCurrentIndex(index);
        }
        on_connBtn_clicked();
    }
}


void MainWindow::on_actSerialNum_triggered()
{
    if(tformSerNum == nullptr)
    {
        tformSerNum = new TFormSerNum(this);
        tformSerNum->setAttribute(Qt::WA_DeleteOnClose);
        connect(tformSerNum, &TFormSerNum::destroyed, this, &MainWindow::onTFormDestroyed);
    }
    tformSerNum->show();
}

