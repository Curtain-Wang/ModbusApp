#include "headfile.h"
#include <mainwindow.h>

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
quint8 queryStep = 0;
uint16_t g_TelRegs[NUM_REGISTER];
uint8_t g_TelRegsPows[NUM_REGISTER] = {2, 1, 2, 1, 0, 0, 2, 2};
uint16_t g_TempTelRegs[NUM_REGISTER];
uint8_t g_TempTelRegsPows[NUM_REGISTER] = {1, 1, 1, 1, 1, 1, 1, 1};
uint16_t g_StatRegs[NUM_REGISTER];
uint8_t g_StatRegsPows[NUM_REGISTER] = {0};
uint16_t g_ParallelRegs[NUM_REGISTER];
uint8_t g_ParallelRegsPows[NUM_REGISTER] = {0, 0, 0, 0, 1, 1};
uint16_t g_ProductRegs[NUM_REGISTER];
uint8_t g_ProductRegsPows[NUM_REGISTER] = {0};
uint16_t g_ChgCfgRegs[NUM_REGISTER];
uint8_t g_ChgCfgRegsPows[NUM_REGISTER] = {2, 1, 0};
uint16_t g_DsgCfgRegs[NUM_REGISTER];
uint8_t g_DsgCfgRegsPows[NUM_REGISTER] = {0, 2, 1, 0};
uint16_t g_ProtectCfgRegs[NUM_REGISTER];
uint8_t g_ProtectCfgRegsPows[NUM_REGISTER] = {2, 2, 1, 0, 2, 2, 1, 0, 0, 0};
uint16_t g_SysCtrlgRegs[NUM_REGISTER];
uint8_t g_SysCtrlgRegsPows[NUM_REGISTER] = {0};
QString g_RunStatus[6] = {"休眠", "待机", "独立放电", "均流放电", "充电", "故障"};
uint8_t g_ConfigGetFlag = 0;
