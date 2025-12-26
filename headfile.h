#ifndef HEADFILE_H
#define HEADFILE_H
#include <QString>

#define INPUT_REG_START_ADDR    0
#define HOLDING_REG_START_ADDR  1000
#define REG_NUM         100
#define TITLE   "LED电源-V1.0.0"
#define MODULE  0x01
#define INPUT_REG_START     0
#define HOLDING_REG_START   1000
#define INPUT_REG_NUM       0x14
#define HOLDING_REG_NUM     18
#define DATA_REFRESH_CYCLE  10
#define NO_WARN_PROT_STR    "无保护事件"
#define UNCONNECTED     0
#define CONNECTING      1
#define CONNECTED       2
#define CONFIG_FILE_PATH  "./config.ini"
#define BASE_CONFIG  "BASE_CONFIG"
#define DOWNLOAD_FILE_DIR "download_file_dir"
#define RED_BUTTON_STYLE            "QPushButton { background-color: #EF5350; border: 2px solid #E53935; color: white; font-size: 25px; padding: 10px; border-radius: 10px; width: 100px; height: 50px; text-align: center; }"
#define GREEN_BUTTON_STYLE          "QPushButton { background-color: #66BB6A; border: 2px solid #43A047; color: white; font-size: 22px; padding: 10px; border-radius: 10px; width: 100px; height: 50px; text-align: center; }"

typedef enum
{
    READ_HOLDING_CMD = 0x03,
    READ_INPUT_CMD = 0x04,
    WRITE_ONE_CMD = 0x06,
    MASTER_CMD = 0xF0,
    SLAVE_CMD = 0xE0,
}en_cmd1_t;

typedef enum
{
    UPDATE_CMD = 0x06,
    DOWNLOAD_DATA_CMD = 0x07,
    DOWNLOAD_COMPLETE_CHECK_CMD = 0x08,
}en_cmd2_t;


#define BR 9600
class MainWindow;

extern qint16 inputRegs[REG_NUM];
extern quint8 inputPow[REG_NUM];
extern qint16 holdingRegs[REG_NUM];
extern quint8 holdingPow[REG_NUM];
//0未连接 1连接中 2已连接
extern int connFlag;
extern QString connStatus;
extern QString versionStr;
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

#endif // HEADFILE_H
