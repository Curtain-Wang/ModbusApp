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
#include <QtCharts>
#include "voltcurchart.h"
#include "tformcali.h"
#include "tformcdtest.h"


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
    , voltCurChart(new VoltCurChart(this))
    , voltCurChartB(new VoltCurChart(this))

{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icon/images/app.ico"));
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete connectStatusLabel;
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
    quint8 index = 0;
    // 遍历可用串口，将串口名添加到 comboBox中
    for(quint8 i = 0; i < portList.size(); i++)
    {
        ui->comboBox_2->addItem(portList[i].portName());
        if(portList[i].portName() == portName)
        {
            index = i;
        }
    }
    ui->comboBox_2->setCurrentIndex(index);
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
    versionLabel->setMidLineWidth(100);
    ui->statusbar->addWidget(versionLabel);
    //持续运行时间
    runTimeLabel->setMidLineWidth(800);
    runTimeLabel->setText(runTimeStr.arg(lastRunSecond / 3600).arg(lastRunSecond % 3600 / 60).arg(lastRunSecond % 60));
    // 设置标签右对齐
    runTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->statusbar->addPermanentWidget(runTimeLabel, 1);  // 第二个参数是伸缩因子
    setWindowTitle(QString(TITLE).arg(serNum));
    //指示灯定时器相关
    txResetTimer->setSingleShot(true);
    connect(txResetTimer, &QTimer::timeout, this, &MainWindow::on_txResetTimer_timeout);
    rxResetTimer->setSingleShot(true);
    connect(rxResetTimer, &QTimer::timeout, this, &MainWindow::on_rxResetTimer_timeout);
    // 确保控件可以获得焦点
    setFocusPolicy(Qt::StrongFocus);
    // 并且实际获得了焦点
    setFocus();

    connect(saveDataTimer, &QTimer::timeout, this, &MainWindow::on_saveDataTimer_timeout);
    saveDataTimer->setInterval(dataRecordCycle * 1000);
    saveDataTimer->start();
    connect(reconnectTimer, &QTimer::timeout, this, &MainWindow::on_reconnectTimer_timeout);
    reconnectTimer->setInterval(5000);
    menuBar()->setStyleSheet("QMenu { background-color: white; }"
                             "QMenu::item { color: black; padding: 5px 20px; }"
                             "QMenu::item:selected {color: black; border: 1px solid gray;}"
                             "QMenu::item:hover {color: black; border: 1px solid gray;}");
    voltCurChartInit();
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


    quint16 startAddr = 0;
    quint16 num = 0;

    switch(queryStep)
    {
    case INPUT_STEP_TEL:
        buf.append(READ_INPUT_CMD);
        startAddr = MODBUS_BLOCK_START_TEL;
        num = MODBUS_BLOCK_SIZE_TEL;
        break;
    case INPUT_STEP_TEMP:
        buf.append(READ_INPUT_CMD);
        startAddr = MODBUS_BLOCK_START_TEMP;
        num = MODBUS_BLOCK_SIZE_TEMP;
        break;
    case INPUT_STEP_STATUS:
        buf.append(READ_INPUT_CMD);
        startAddr = MODBUS_BLOCK_START_STAT;
        num = MODBUS_BLOCK_SIZE_STAT;
        break;
    case INPUT_STEP_PARALLEL:
        buf.append(READ_INPUT_CMD);
        startAddr = MODBUS_BLOCK_START_PARALLEL;
        num = MODBUS_BLOCK_SIZE_PARALLEL;
        break;
    case INPUT_STEP_PROD:
        buf.append(READ_INPUT_CMD);
        startAddr = MODBUS_BLOCK_START_PRODUCT;
        num = MODBUS_BLOCK_SIZE_PRODUCT;
        break;
    case HOLDING_STEP_CHG_CFG:
        buf.append(READ_HOLDING_CMD);
        startAddr = MODBUS_BLOCK_START_CHARGE;
        num = MODBUS_BLOCK_SIZE_CHARGE;
        break;
    case HOLDING_STEP_DSG_CFG:
        buf.append(READ_HOLDING_CMD);
        startAddr = MODBUS_BLOCK_START_DISCHARGE;
        num = MODBUS_BLOCK_SIZE_DISCHARGE;
        break;
    case HOLDING_STEP_PROT_CFG:
        buf.append(READ_HOLDING_CMD);
        startAddr = MODBUS_BLOCK_START_PROTECT;
        num = MODBUS_BLOCK_SIZE_PROTECT;
        break;
    case HOLDING_STEP_SYS_CTRL_CFG:
        buf.append(READ_HOLDING_CMD);
        startAddr = MODBUS_BLOCK_START_CTRL;
        num = MODBUS_BLOCK_SIZE_CTRL;
        break;
    default:
        queryStep = INPUT_STEP_TEL;
        buf.append(READ_INPUT_CMD);
        startAddr = MODBUS_BLOCK_START_TEL;
        num = MODBUS_BLOCK_SIZE_TEL;
        break;
    }

    //起始地址
    buf.append(static_cast<char>(startAddr >> 8));
    buf.append(static_cast<char>(startAddr & 0xFF));
    //个数
    buf.append(static_cast<char>(num >> 8));
    buf.append(static_cast<char>(num & 0xFF));

    if((tformConfig1 != nullptr && queryStep == HOLDING_STEP_SYS_CTRL_CFG)
        || (g_ConfigGetFlag && tformConfig1 == nullptr && queryStep == INPUT_STEP_PROD))
    {
        dataRefreshRemaingTime = DATA_REFRESH_CYCLE;
    }

    QByteArray crcArray = calculateCRCArray(buf, 6);
    //crC
    buf.append(crcArray[0]);
    buf.append(crcArray[1]);
    sendPortData(buf);
    //设置等待时间
    waitMessageRemaingTime = 20;
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
            char c = static_cast<char>(byte);
            receiveDataBuf.replace(receiveEndIndex, 1, &c, 1);
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
    if(connFlag != CONNECTED)
    {
        connFlag = CONNECTED;
        voltCurChart->start();
        voltCurChartB->start();
        g_ConfigGetFlag = 0;
    }
    timeoutTimes = 0;
    connectStatusLabel->setText(connStatus.arg("已连接"));
    connectStatusLabel->setStyleSheet("QLabel { background-color : green; color : white; }");
    //查询命令
    if(data[1] == READ_INPUT_CMD || data[1] == READ_HOLDING_CMD)
    {
        // 根据 inputStep 选择对应的目标寄存器数组
        quint16* targetArray = nullptr;
        switch (queryStep) {
        case INPUT_STEP_TEL:
            targetArray = g_TelRegs;
            break;
        case INPUT_STEP_TEMP:
            targetArray = g_TempTelRegs;
            break;
        case INPUT_STEP_STATUS:
            targetArray = g_StatRegs;
            break;
        case INPUT_STEP_PARALLEL:
            targetArray = g_ParallelRegs;
            break;
        case INPUT_STEP_PROD:
            targetArray = g_ProductRegs;
            break;
        case HOLDING_STEP_CHG_CFG:
            targetArray = g_ChgCfgRegs;
            break;
        case HOLDING_STEP_DSG_CFG:
            targetArray = g_DsgCfgRegs;
            break;
        case HOLDING_STEP_PROT_CFG:
            targetArray = g_ProtectCfgRegs;
            break;
        case HOLDING_STEP_SYS_CTRL_CFG:
            targetArray = g_SysCtrlgRegs;
            g_ConfigGetFlag = 1;
            break;
        default: return; // 无效步骤直接返回（根据实际需求调整）
        }

        // 统一处理数据复制（避免重复循环逻辑）
        const quint16 regCount = data[2] / 2; // 提前计算避免重复除法
        for (quint16 i = 0; i < regCount; ++i) {
            // 安全处理字节序：高位字节 << 8 | 低位字节
            targetArray[i] = (static_cast<quint16>(data[3 + i * 2]) << 8)
                             | data[4 + i * 2];
        }

        if(queryStep < HOLDING_STEP_CHG_CFG)
        {
            refresh();
        }else
        {
            if(tformConfig1 != nullptr)
            {
                tformConfig1->refresh();
            }
            if(tformCali != nullptr)
            {
                tformCali->refresh();
            }
        }

        if(tformConfig1 != nullptr || g_ConfigGetFlag == 0 || tformCali != nullptr)
        {
            queryStep = (queryStep + 1) % 9; //更新步骤
        }else
        {
            queryStep = (queryStep + 1) % 5; //更新步骤
        }
    }
    //下载命令
    if(data[1] == DOWNLOAD_CMD && tformDownload != nullptr)
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
    if(tformConfig1 != nullptr)
        tformConfig1->refresh();
}

void MainWindow::refresh()
{
    //遥测
    ui->tel0->setText(QString::number(static_cast<float>(g_TelRegs[0] * 1.0 / qPow(10, g_TelRegsPows[0])), 'f', g_TelRegsPows[0]));
    ui->tel1->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TelRegs[1]) * 1.0 / qPow(10, g_TelRegsPows[1])), 'f', g_TelRegsPows[1]));
    ui->tel2->setText(QString::number(static_cast<float>(g_TelRegs[2] * 1.0 / qPow(10, g_TelRegsPows[2])), 'f', g_TelRegsPows[2]));
    ui->tel3->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TelRegs[3]) * 1.0 / qPow(10, g_TelRegsPows[3])), 'f', g_TelRegsPows[3]));
    ui->tel4->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TelRegs[4]) * 1.0 / qPow(10, g_TelRegsPows[4])), 'f', g_TelRegsPows[4]));
    ui->tel5->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TelRegs[5]) * 1.0 / qPow(10, g_TelRegsPows[5])), 'f', g_TelRegsPows[5]));
    ui->tel6->setText(QString::number(static_cast<float>(g_TelRegs[6] * 1.0 / qPow(10, g_TelRegsPows[6])), 'f', g_TelRegsPows[6]));
    ui->tel7->setText(QString::number(static_cast<float>(g_TelRegs[7] * 1.0 / qPow(10, g_TelRegsPows[7])), 'f', g_TelRegsPows[7]));
    ui->ph->setText(QString::number(static_cast<float>(g_TelRegs[8] * 1.0 / qPow(10, g_TelRegsPows[8])), 'f', g_TelRegsPows[8]));
    ui->bh->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TelRegs[9]) * 1.0 / qPow(10, g_TelRegsPows[9])), 'f', g_TelRegsPows[9]));
    ui->verr->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TelRegs[10]) * 1.0 / qPow(10, g_TelRegsPows[10])), 'f', g_TelRegsPows[10]));
    ui->iref->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TelRegs[11]) * 1.0 / qPow(10, g_TelRegsPows[11])), 'f', g_TelRegsPows[11]));
    ui->fchg_rem_time->setText(QString::number(static_cast<float>(g_TelRegs[12] * 1.0 / qPow(10, g_TelRegsPows[12])), 'f', g_TelRegsPows[12]));
    //温度遥测
    ui->temp0->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TempTelRegs[0]) * 1.0 / qPow(10, g_TempTelRegsPows[0])), 'f', g_TempTelRegsPows[0]));
    ui->temp1->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TempTelRegs[1]) * 1.0 / qPow(10, g_TempTelRegsPows[1])), 'f', g_TempTelRegsPows[1]));
    ui->temp2->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TempTelRegs[2]) * 1.0 / qPow(10, g_TempTelRegsPows[2])), 'f', g_TempTelRegsPows[2]));
    ui->temp3->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TempTelRegs[3]) * 1.0 / qPow(10, g_TempTelRegsPows[3])), 'f', g_TempTelRegsPows[3]));
    ui->temp4->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TempTelRegs[4]) * 1.0 / qPow(10, g_TempTelRegsPows[4])), 'f', g_TempTelRegsPows[4]));
    ui->temp5->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TempTelRegs[5]) * 1.0 / qPow(10, g_TempTelRegsPows[5])), 'f', g_TempTelRegsPows[5]));
    ui->temp6->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TempTelRegs[6]) * 1.0 / qPow(10, g_TempTelRegsPows[6])), 'f', g_TempTelRegsPows[6]));
    ui->temp7->setText(QString::number(static_cast<float>(static_cast<qint16>(g_TempTelRegs[7]) * 1.0 / qPow(10, g_TempTelRegsPows[7])), 'f', g_TempTelRegsPows[7]));
    if(g_StatRegs[4] < 6)
        ui->run_status->setText(g_RunStatus[g_StatRegs[4]]);

    QString eventStr = getEventText(g_StatRegs[0], g_StatRegs[1], g_StatRegs[2], g_StatRegs[3]);
    if(g_StatRegs[0] + g_StatRegs[1] + g_StatRegs[2] + g_StatRegs[3] == 0)
    {
        ui->bms_warn_prot->setText(NO_WARN_PROT_STR);
        ui->bms_warn_prot->setProperty("status", "normal");
    }else if(g_StatRegs[1] > 0 || (g_StatRegs[0] & 0x75F) > 0)
    {
        ui->bms_warn_prot->setText(eventStr);
        ui->bms_warn_prot->setProperty("status", "prot");
    }else
    {
        ui->bms_warn_prot->setText(eventStr);
        ui->bms_warn_prot->setProperty("status", "alarm");
    }

    ui->bms_warn_prot->style()->unpolish(ui->bms_warn_prot);
    ui->bms_warn_prot->style()->polish(ui->bms_warn_prot);
    ui->bms_warn_prot->update();

    ui->parallel0->setText(QString::number(g_ParallelRegs[0]));
    ui->parallel1->setText(QString::number(g_ParallelRegs[1]));
    ui->parallel2->setText(QString::number(g_ParallelRegs[2]));
    ui->parallel3->setText(g_ParallelRegs[3] == 1 ? "主" : "从");
    ui->parallel4->setText(QString::number(static_cast<float>(static_cast<qint16>(g_ParallelRegs[4]) * 1.0 / qPow(10,g_ParallelRegsPows[4])), 'f', g_ParallelRegsPows[4]));
    ui->parallel5->setText(QString::number(static_cast<float>(static_cast<qint16>(g_ParallelRegs[5]) * 1.0 / qPow(10, g_ParallelRegsPows[5])), 'f', g_ParallelRegsPows[5]));
    versionLabel->setText(versionStr.arg(getSoftVersion(g_ProductRegs[4])));
    serNum = QString::number(g_SysCtrlgRegs[28] + (g_SysCtrlgRegs[29] << 16));
    setWindowTitle(QString(TITLE).arg(serNum));

    //B侧继电器
    if((g_StatRegs[5] & 1) == 1)
    {
        ui->lab_br->setStyleSheet(
            "QLabel {"
            "    border-radius: 10px;"
            "    background-color: #00F000;"
            "}"
            );
    }else
    {
        ui->lab_br->setStyleSheet(
            "QLabel {"
            "    border-radius: 10px;"
            "    background-color: #D3D3D3;"
            "}"
            );
    }

    //P侧继电器
    if(((g_StatRegs[5] >> 1) & 1) == 1)
    {
        ui->lab_pr->setStyleSheet(
            "QLabel {"
            "    border-radius: 10px;"
            "    background-color: #00F000;"
            "}"
            );
    }else
    {
        ui->lab_pr->setStyleSheet(
            "QLabel {"
            "    border-radius: 10px;"
            "    background-color: #D3D3D3;"
            "}"
            );
    }
}

QString MainWindow::getEventText(quint16 fault1, quint16 fault2, quint16 warn1, quint16 warn2)
{
    QString text;
    if((fault1 & 1) == 1)   text.append("短路保护、");
    if(((fault1 >> 1) & 1) == 1)   text.append("超温保护、");
    if(((fault1 >> 2) & 1) == 1)   text.append("充电过流保护、");
    if(((fault1 >> 3) & 1) == 1)   text.append("放电过流保护、");
    if(((fault1 >> 4) & 1) == 1)   text.append("电池侧过压保护、");
    if(((fault1 >> 5) & 1) == 1)   text.append("电池侧欠压保护、");
    if(((fault1 >> 6) & 1) == 1)   text.append("逆变侧过压保护、");
    if(((fault1 >> 7) & 1) == 1)   text.append("逆变侧欠压保护、");
    if(((fault1 >> 8) & 1) == 1)   text.append("CMP保护、");
    if(((fault1 >> 9) & 1) == 1)   text.append("B侧内外压差不平衡、");
    if(((fault1 >> 10) & 1) == 1)   text.append("P侧内外压差不平衡、");
    if(((fault1 >> 11) & 1) == 1)   text.append("过功率保护、");


    if((fault2 & 1) == 1)   text.append("熔断器故障、");
    if(((fault2 >> 1) & 1) == 1)   text.append("系统错误、");
    if(((fault2 >> 2) & 1) == 1)   text.append("通信故障、");
    if(((fault2 >> 3) & 1) == 1)   text.append("CMP P侧过流保护、");
    if(((fault2 >> 4) & 1) == 1)   text.append("CMP B侧过流保护、");
    if(((fault2 >> 5) & 1) == 1)   text.append("CMP P侧过压保护、");
    if(((fault2 >> 6) & 1) == 1)   text.append("CMP B侧过压保护、");
    if(((fault2 >> 7) & 1) == 1)   text.append("保护锁定、");



    if((warn1 & 1) == 1)   text.append("充电过流告警、");
    if(((warn1 >> 1) & 1) == 1)   text.append("放电过流告警、");
    if(((warn1 >> 2) & 1) == 1)   text.append("电池侧欠压告警、");
    if(((warn1 >> 3) & 1) == 1)   text.append("电池侧过压告警、");
    if(((warn1 >> 4) & 1) == 1)   text.append("逆变侧欠压告警、");
    if(((warn1 >> 5) & 1) == 1)   text.append("逆变侧过压告警、");

    if((warn2 & 1) == 1)   text.append("高温告警、");

    if(text.length() > 0)
    {
        text.removeAt(text.length() - 1);
    }
    return text;
}

QString MainWindow::getSoftVersion(quint16 sftVer)
{
    // 转换为16进制字符串
    QString hexStr = QString::number(sftVer, 16);  // 转换为16进制字符串 "112"

    //补齐四个字符
    quint8 size = 4 - hexStr.length();
    for(quint8 i = 0; i < size; i++)
    {
        hexStr = "0" + hexStr;
    }

    // 提取版本号的高位和低位部分
    QString major = hexStr.mid(0, 2);
    QString minor = hexStr.mid(2, 2);

    return QString("V%1.%2").arg(major).arg(minor);
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

void MainWindow::manualWriteTwoRegBuild(quint16 addr, quint16 value1, quint16 value2)
{
    if(manualFlag == 1)
    {
        QMessageBox::information(this, "冲突", "当前有其他手动命令在发送, 请稍后再试!");
        return;
    }
    manualSendDataBuf.clear();
    manualSendDataBuf.append(MODULE);
    manualSendDataBuf.append(WRITE_MULTI_CMD);
    manualSendDataBuf.append(addr >> 8);
    manualSendDataBuf.append(addr & 0xFF);
    manualSendDataBuf.append(static_cast<char>(0));
    manualSendDataBuf.append(2);
    manualSendDataBuf.append(4);
    manualSendDataBuf.append(value1 >> 8);
    manualSendDataBuf.append(value1 & 0xFF);
    manualSendDataBuf.append(value2 >> 8);
    manualSendDataBuf.append(value2 & 0xFF);
    QByteArray crcArray = calculateCRCArray(manualSendDataBuf, 11);
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
    int cmd2 = static_cast<uint8_t>(receiveDataBuf[(receiveStartIndex + 3) % 500]);
    quint16 len = 0;
    if(cmd == READ_HOLDING_CMD || cmd == READ_INPUT_CMD)
    {
        len = receiveDataBuf[(receiveStartIndex + 2) % 500] + 5;
    }
    if(cmd == WRITE_ONE_CMD)
    {
        len = 8;
    }
    if(cmd == DOWNLOAD_CMD)
    {
        if(cmd2 == SHAKE_HANDS_CMD || cmd2 == ERASURE_CMD || cmd2 == FINISH_CMD)
        {
            len = 7;
        }
        if(cmd2 == WRITE_BLOCK_CMD)
        {
            len = 9;
        }
    }
    if(len > 260)
    {
        len = 0;
    }
    return len;
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
    headers.append("累计运行时间");
    headers.append("运行模式");
    headers.append("告警保护事件");
    headers.append("电池侧外侧电压(V)");
    headers.append("电池侧内侧电压(V)");
    headers.append("电池侧电流(A)");
    headers.append("电池侧功率(W)");
    headers.append("逆变侧外侧电压(V)");
    headers.append("逆变侧内侧电压(V)");
    headers.append("逆变侧电流(A)");
    headers.append("逆变侧功率(W)");
    headers.append("B侧MOS温度(℃)");
    headers.append("P侧MOS温度(℃)");
    headers.append("环境温度(℃)");
    headers.append("散热器温度(℃)");
    headers.append("功率电感1温度(℃)");
    headers.append("功率电感2温度(℃)");
    headers.append("功率电感3温度(℃)");
    headers.append("功率电感4温度(℃)");
    out << headers.join(",") << "\n";
}

void MainWindow::writeDataToCSV(QTextStream &out, const QDateTime &currentTime)
{
    QStringList data;
    data << currentTime.toString("yyyy-MM-dd hh:mm:ss");
    data << QString("%1时%2分%3秒").arg(runSecond / 36000).arg(runSecond % 36000 / 600).arg(runSecond % 600 / 10);
    if(g_StatRegs[4] < 6)
    {
        data << g_RunStatus[g_StatRegs[4]];
    }
    else
    {
        data << "-";
    }
    //告警/保护
    QString eventStr = getEventText(g_StatRegs[0], g_StatRegs[1], g_StatRegs[2], g_StatRegs[3]);

    if(eventStr.isEmpty())
    {
        data << "无事件";
    }else
    {
        data << eventStr;
    }

    //电池侧外侧电压(V)
    data << QString::number(static_cast<float>(g_TelRegs[0] * 1.0 / qPow(10, g_TelRegsPows[0])), 'f', g_TelRegsPows[0]);
    //电池侧内侧电压(V)
    data << QString::number(static_cast<float>(g_TelRegs[6] * 1.0 / qPow(10, g_TelRegsPows[6])), 'f', g_TelRegsPows[6]);
    //电池侧电流
    data << QString::number(static_cast<float>(static_cast<qint16>(g_TelRegs[1]) * 1.0 / qPow(10, g_TelRegsPows[1])), 'f', g_TelRegsPows[1]);
    //电池侧功率
    data << QString::number(static_cast<float>(static_cast<qint16>(g_TelRegs[4]) * 1.0 / qPow(10, g_TelRegsPows[4])), 'f', g_TelRegsPows[4]);
    //逆变侧外侧电压
    data << QString::number(static_cast<float>(g_TelRegs[2] * 1.0 / qPow(10, g_TelRegsPows[2])), 'f', g_TelRegsPows[2]);
    //逆变侧内侧电压
    data << QString::number(static_cast<float>(g_TelRegs[7] * 1.0 / qPow(10, g_TelRegsPows[7])), 'f', g_TelRegsPows[7]);
    //逆变侧电流(A)
    data << QString::number(static_cast<float>(static_cast<qint16>(g_TelRegs[3]) * 1.0 / qPow(10, g_TelRegsPows[3])), 'f', g_TelRegsPows[3]);
    //逆变侧功率(W)
    data << QString::number(static_cast<float>(static_cast<qint16>(g_TelRegs[5]) * 1.0 / qPow(10, g_TelRegsPows[5])), 'f', g_TelRegsPows[5]);
    //B侧MOS温度(℃)
    data << QString::number(static_cast<float>(g_TempTelRegs[0] * 1.0 / qPow(10, g_TempTelRegsPows[0])), 'f', g_TempTelRegsPows[0]);
    //P侧MOS温度(℃)
    data << QString::number(static_cast<float>(g_TempTelRegs[1] * 1.0 / qPow(10, g_TempTelRegsPows[1])), 'f', g_TempTelRegsPows[1]);
    //环境温度
    data << QString::number(static_cast<float>(g_TempTelRegs[2] * 1.0 / qPow(10, g_TempTelRegsPows[2])), 'f', g_TempTelRegsPows[2]);
    //散热器温度
    data << QString::number(static_cast<float>(g_TempTelRegs[3] * 1.0 / qPow(10, g_TempTelRegsPows[3])), 'f', g_TempTelRegsPows[3]);
    //功率电感1温度(℃)
    data << QString::number(static_cast<float>(g_TempTelRegs[4] * 1.0 / qPow(10, g_TempTelRegsPows[4])), 'f', g_TempTelRegsPows[4]);
    //功率电感2温度(℃)
    data << QString::number(static_cast<float>(g_TempTelRegs[5] * 1.0 / qPow(10, g_TempTelRegsPows[5])), 'f', g_TempTelRegsPows[5]);
    //功率电感3温度
    data << QString::number(static_cast<float>(g_TempTelRegs[6] * 1.0 / qPow(10, g_TempTelRegsPows[6])), 'f', g_TempTelRegsPows[6]);
    //功率电感4温度
    data << QString::number(static_cast<float>(g_TempTelRegs[7] * 1.0 / qPow(10, g_TempTelRegsPows[7])), 'f', g_TempTelRegsPows[7]);
    out << data.join(",") << "\n";
}

void MainWindow::runTimeDeal()
{
    //未连接、未启动、有事件
    if(connFlag != CONNECTED
        || (g_StatRegs[4] != 2 && g_StatRegs[4] != 3 && g_StatRegs[4] != 4)
        || g_StatRegs[0] >0 || g_StatRegs[1] > 0)
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

void MainWindow::voltCurChartInit()
{
    // 设置为显示最近10秒的数据
    voltCurChart->setTimeWindow(10);

    voltCurChart->setVoltageRange(0, 70);
    voltCurChart->setCurrentRange(0, 200);

    // 清除容器中的现有布局（如果有的话）
    if (ui->chartContainer->layout()) {
        QLayoutItem *child;
        while ((child = ui->chartContainer->layout()->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
        delete ui->chartContainer->layout();
    }

    // 创建新布局并将图表添加进去
    QVBoxLayout *layout = new QVBoxLayout(ui->chartContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(voltCurChart);
    ui->chartContainer->setLayout(layout);

    // 设置为显示最近10秒的数据
    voltCurChartB->setTimeWindow(10);
    voltCurChartB->setVoltageRange(0, 70);
    voltCurChartB->setCurrentRange(0, 200);
    voltCurChartB->swtichB();
    // 清除容器中的现有布局（如果有的话）
    if (ui->chartContainerB->layout()) {
        QLayoutItem *child;
        while ((child = ui->chartContainerB->layout()->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
        delete ui->chartContainerB->layout();
    }

    // 创建新布局并将图表添加进去
    QVBoxLayout *layoutB = new QVBoxLayout(ui->chartContainerB);
    layoutB->setContentsMargins(0, 0, 0, 0);
    layoutB->setSpacing(0);
    layoutB->addWidget(voltCurChartB);
    ui->chartContainerB->setLayout(layoutB);
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
        if(module != MODULE || (cmd != READ_HOLDING_CMD && cmd != WRITE_ONE_CMD && cmd != READ_INPUT_CMD && cmd != DOWNLOAD_CMD))
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
    if(obj == tformCali)
    {
        tformCali = nullptr;
    }
    if(obj == tformCDTest)
    {
        tformCDTest = nullptr;
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

void MainWindow::on_saveDataTimer_timeout()
{
    //只记录启动状态下的数据
    if(connFlag == CONNECTED)
    {
        QDateTime currentTime = QDateTime::currentDateTime();
        QString serialNumberStr = serNum.remove(QChar('\0')).trimmed();
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
            bool flag = dir.mkpath(".");
            if(tform1 != nullptr && flag)
            {
                tform1->displayInfo("未找到路径，已创建");
            }

        }
        filePath = QString("%1/Save/%2/%3.csv")
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
            // 添加 UTF-8 BOM
            csvFile.write("\xEF\xBB\xBF");
            initializeCSVFile(out);
        }
        writeDataToCSV(out, currentTime);
        csvFile.flush();

    }
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
        }else
        {
            return;//没找到目标串口直接返回
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


void MainWindow::on_actClrFault_triggered()
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    mainwindow->manualWriteOneCMDBuild(0x4301, 1);
}


void MainWindow::on_actForceChg_triggered()
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    mainwindow->manualWriteOneCMDBuild(0x4301, (1 << 1));
}


void MainWindow::on_actHib_triggered()
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    mainwindow->manualWriteOneCMDBuild(0x4301, (1 << 2));
}


void MainWindow::on_actIndDsg_triggered()
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    mainwindow->manualWriteOneCMDBuild(0x4301, (1 << 3));
}


void MainWindow::on_actUniDsg_triggered()
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    mainwindow->manualWriteOneCMDBuild(0x4301, (1 << 4));
}


void MainWindow::on_actChg_triggered()
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    mainwindow->manualWriteOneCMDBuild(0x4301, (1 << 5));
}


void MainWindow::on_actStandby_triggered()
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    mainwindow->manualWriteOneCMDBuild(0x4301, (1 << 6));
}


void MainWindow::on_actPowOff_triggered()
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    mainwindow->manualWriteOneCMDBuild(0x4301, (1 << 7));
}


void MainWindow::on_pushButton_9_clicked()
{
    if(tformCali == nullptr)
    {
        tformCali = new TFormCali(this);
        tformCali->setAttribute(Qt::WA_DeleteOnClose);
        connect(tformCali, &TFormCali::destroyed, this, &MainWindow::onTFormDestroyed);
    }
    tformCali->show();
}


void MainWindow::on_actionCDTest_triggered()
{
    if(tformCDTest == nullptr)
    {
        tformCDTest = new TFormCDTest(this);
        tformCDTest->setAttribute(Qt::WA_DeleteOnClose);
        connect(tformCDTest, &TFormCDTest::destroyed, this, &MainWindow::onTFormDestroyed);
    }
    tformCDTest->show();
}


void MainWindow::on_actParallelChg_triggered()
{
    if(connFlag != CONNECTED)
    {
        QMessageBox::information(this, tr("提示"), tr("请先建立连接!"));
        return;
    }
    mainwindow->manualWriteOneCMDBuild(0x4301, (1 << 8));
}

