#include "headfile.h"
#include <mainwindow.h>

qint16 inputRegs[REG_NUM];
qint16 holdingRegs[REG_NUM];
quint8 inputPow[REG_NUM];
quint8 holdingPow[REG_NUM];

int connFlag = 0;
QString connStatus = "连接状态：%1";
QString versionStr = "版本：%1.%2";
QString runTimeStr = "最近一次持续运行时间：%1时 %2分 %3秒";
int waitMessageRemaingTime = 0;
int dataRefreshRemaingTime = 0;
//手动标记，0自动 1手动 2双手动
int manualFlag = 0;
QByteArray manualSendDataBuf;
QByteArray receiveDataBuf(500, 0);
int receiveStartIndex = 0;
int receiveEndIndex = 0;
quint16 lastEditAddr = 0;
MainWindow* mainwindow = nullptr;
int DownloadFlag = 0;
QByteArray rxBuf;
quint16 lastMSCommCount = 0;
quint32 dataRecordCycle = 0;
QString dataRecordFilePath = "";
//重连标记, 0未重连1正在重连
quint8 reconnectFlag = 0;
int baudRate = 0;
QString portName = "";
//超时次数
quint8 timeoutTimes = 0;
QString batSerNum = "未知";
quint64 lastRunSecond = 0;
quint64 runSecond = 0;
