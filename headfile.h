#ifndef HEADFILE_H
#define HEADFILE_H
#include <QString>

#define INPUT_REG_START_ADDR    0
#define HOLDING_REG_START_ADDR  1000
#define REG_NUM         100
#define TITLE   "LED电源-V1.0.0"
#define MODULE  0x01
#define READ_HOLDING_CMD  0x03
#define READ_INPUT_CMD  0x04
#define WRITE_ONE_CMD   0x06
#define INPUT_REG_START     0
#define HOLDING_REG_START   1000
#define INPUT_REG_NUM       0x14
#define HOLDING_REG_NUM     17
#define DATA_REFRESH_CYCLE  20
#define NO_WARN_PROT_STR    "无保护事件"
#define UNCONNECTED     0
#define CONNECTING      1
#define CONNECTED       2


#define BR 9600
class MainWindow;

extern qint16 inputRegs[REG_NUM];
extern quint8 inputPow[REG_NUM];
extern qint16 holdingRegs[REG_NUM];
extern quint8 holdingPow[REG_NUM];
//0未连接 1连接中 2已连接
extern int connFlag;
extern QString connStatus;
extern int waitMessageRemaingTime;
extern int dataRefreshRemaingTime;
//手动标记，0自动 1手动 2双手动
extern int manualFlag;
extern QByteArray manualSendDataBuf;
extern QByteArray receiveDataBuf;
extern int receiveStartIndex;
extern int receiveEndIndex;
extern quint16 lastEditAddr;
extern MainWindow* mainwindow;

#endif // HEADFILE_H
