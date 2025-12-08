#include "headfile.h"
#include <mainwindow.h>

qint16 inputRegs[REG_NUM];
qint16 holdingRegs[REG_NUM];
quint8 inputPow[REG_NUM];
quint8 holdingPow[REG_NUM];

int connFlag = 0;
QString connStatus = "连接状态：%1";
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
