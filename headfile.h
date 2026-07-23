#ifndef HEADFILE_H
#define HEADFILE_H
#include <QString>

#define INPUT_REG_START_ADDR    0
#define HOLDING_REG_START_ADDR  1000
#define REG_NUM         100
#define TITLE   "DPS4K2-V0.0.2-%1"
#define MODULE  0x01
#define INPUT_REG_START     0
#define HOLDING_REG_START   1000
#define INPUT_REG_NUM       30
#define HOLDING_REG_NUM     21
#define DATA_REFRESH_CYCLE  10
#define NO_WARN_PROT_STR    "无保护事件"
#define UNCONNECTED     0
#define CONNECTING      1
#define CONNECTED       2
#define CONFIG_FILE_PATH  "./config.ini"
#define BASE_CONFIG  "BASE_CONFIG"
#define DOWNLOAD_FILE_DIR "download_file_dir"
#define RED_BUTTON_STYLE                "QPushButton { background-color: #EF5350; border: 2px solid #E53935; color: white; font-size: 22px; padding: 10px; border-radius: 10px; width: 100px; height: 50px; text-align: center; } QPushButton:hover { background-color: #e14a47; border: 2px solid #D32F2F; } QPushButton:pressed { background-color: #E53935; border: 2px solid #B71C1C; } QPushButton:checked { background-color: #E53935; border: 2px solid #B71C1C; color: #FFEBEE; box-shadow: 0 0 8px rgba(239, 83, 80, 0.6); font-weight: bold; }"
#define GREEN_BUTTON_STYLE              "QPushButton { background-color: #66BB6A; border: 2px solid #43A047; color: white; font-size: 22px; padding: 10px; border-radius: 10px; width: 100px; height: 50px; text-align: center; } QPushButton:hover { background-color: #5AAE5E; border: 2px solid #388E3C; } QPushButton:pressed { background-color: #4CAF50; border: 2px solid #2C6E2E; } QPushButton:checked { background-color: #4CAF50; border: 2px solid #2E7D32; color: #E8F5E9; box-shadow: 0 0 8px rgba(76, 175, 80, 0.6); font-weight: bold; }"
#define DATA_RECORD_CONFIG  "DATA_RECORD_CONFIG"
#define DATA_RECORD_FILE_PATH  "data_record_file_path"
#define DATA_RECORD_CYCLE  "data_record_cycle"
#define DEFAULT_DATA_RECORD_CYCLE  60
#define CONTINUOUS_RUN_TIME    "CONTINUOUS_RUN_TIME"

#define NUM_REGISTER 0xFF
typedef enum
{
    READ_HOLDING_CMD = 0x03,
    READ_INPUT_CMD = 0x04,
    WRITE_ONE_CMD = 0x06,
    WRITE_MULTI_CMD = 0x10,
    DOWNLOAD_CMD = 0xAA,
}en_cmd1_t;

typedef enum
{
    SHAKE_HANDS_CMD = 0x01,
    ERASURE_CMD = 0x02,
    WRITE_BLOCK_CMD = 0x03,
    FINISH_CMD     = 0x04,
}en_cmd2_t;

typedef enum
{
    HI_RunMode           = 0,
    HI_OutVolt,
    HI_OutCur,
    HI_DimingMode,
    HI_FullLightCur, //满亮度电流值
    HI_FanCtrlMode,
    HI_FanPowSet,
    HI_OutLimitVolt,
    HI_OutLimitCur,
    HI_OutOVPTS, //输出过压保护阈值
    HI_OutOCPTS, //输出过流保护阈值
    HI_InOVPTS,  //输入过压保护阈值
    HI_InOCPTS,  //输入过流保护阈值
    HI_InUVPTS,  //输入欠压保护阈值
    HI_InUVPRTS, //输入欠压保护恢复阈值
    HI_OTTS,    //过温阈值
    HI_OHTS,    //过湿阈值
    HI_OPEN,
    HI_MASTER,  //是否主机
}en_modbus_holding_data_index_t;

typedef enum
{
    INPUT_STEP_TEL,
    INPUT_STEP_TEMP,
    INPUT_STEP_STATUS,
    INPUT_STEP_PARALLEL,
    INPUT_STEP_PROD,
    HOLDING_STEP_CHG_CFG,
    HOLDING_STEP_DSG_CFG,
    HOLDING_STEP_PROT_CFG,
    HOLDING_STEP_SYS_CTRL_CFG
}en_query_step_t;

// 每类寄存器的起始地址（用于计算偏移）
typedef enum {
    MODBUS_BLOCK_START_TEL       = 0x3001,  // 遥测数据起始地址
    MODBUS_BLOCK_START_TEMP      = 0x3101,  // 温度遥测起始地址
    MODBUS_BLOCK_START_STAT      = 0x3201,  // 状态与告警起始地址
    MODBUS_BLOCK_START_PARALLEL  = 0x3301,  // 并机状态起始地址
    MODBUS_BLOCK_START_PRODUCT   = 0x3401,  // 产品标识起始地址
    MODBUS_BLOCK_START_CHARGE    = 0x4001,  // 充电参数起始地址
    MODBUS_BLOCK_START_DISCHARGE = 0x4101,  // 放电参数起始地址
    MODBUS_BLOCK_START_PROTECT   = 0x4201,  // 保护参数起始地址
    MODBUS_BLOCK_START_CTRL      = 0x4301,  // 控制指令起始地址
} modbus_block_start_addr_t;

// 每类寄存器的起始地址（用于计算偏移）
typedef enum {
    MODBUS_BLOCK_SIZE_TEL       = 13,  // 遥测寄存器数量
    MODBUS_BLOCK_SIZE_TEMP      = 8,  // 温度遥测寄存器数量
    MODBUS_BLOCK_SIZE_STAT      = 6,  // 状态与告警寄存器数量
    MODBUS_BLOCK_SIZE_PARALLEL  = 6,  // 并机状态寄存器数量
    MODBUS_BLOCK_SIZE_PRODUCT   = 6,  // 产品标识寄存器数量
    MODBUS_BLOCK_SIZE_CHARGE    = 3,  // 充电参数寄存器数量
    MODBUS_BLOCK_SIZE_DISCHARGE = 4,  // 放电参数寄存器数量
    MODBUS_BLOCK_SIZE_PROTECT   = 11,  // 保护参数寄存器数量
    MODBUS_BLOCK_SIZE_CTRL      = 42,  // 控制指令寄存器数量
} modbus_block_size_addr_t;

#define BR 9600
class MainWindow;

//0未连接 1连接中 2已连接
extern int connFlag;
extern QString connStatus;
extern QString versionStr;
extern QString runTimeStr;
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
extern int DownloadFlag;
extern QByteArray rxBuf;
extern quint16 lastMSCommCount;
extern QString dataRecordFilePath;
extern quint32 dataRecordCycle;
extern quint8 reconnectFlag;
extern int baudRate;
extern QString portName;
//超时次数
extern quint8 timeoutTimes;
extern QString serNum;
extern quint64 lastRunSecond;
extern quint64 runSecond;
extern quint8 queryStep;
extern uint16_t g_TelRegs[NUM_REGISTER];
extern uint8_t g_TelRegsPows[NUM_REGISTER];
extern uint16_t g_TempTelRegs[NUM_REGISTER];
extern uint8_t g_TempTelRegsPows[NUM_REGISTER];
extern uint16_t g_StatRegs[NUM_REGISTER];
extern uint8_t g_StatRegsPows[NUM_REGISTER];
extern uint16_t g_ParallelRegs[NUM_REGISTER];
extern uint8_t g_ParallelRegsPows[NUM_REGISTER];
extern uint16_t g_ProductRegs[NUM_REGISTER];
extern uint8_t g_ProductRegsPow[NUM_REGISTER];
extern uint16_t g_ChgCfgRegs[NUM_REGISTER];
extern uint8_t g_ChgCfgRegsPows[NUM_REGISTER];
extern uint16_t g_DsgCfgRegs[NUM_REGISTER];
extern uint8_t g_DsgCfgRegsPows[NUM_REGISTER];
extern uint16_t g_ProtectCfgRegs[NUM_REGISTER];
extern uint8_t g_ProtectCfgRegsPows[NUM_REGISTER];
extern uint16_t g_SysCtrlgRegs[NUM_REGISTER];
extern uint8_t g_SysCtrlgRegsPows[NUM_REGISTER];
extern QString g_RunStatus[6];
extern uint8_t g_ConfigGetFlag;
#endif // HEADFILE_H
