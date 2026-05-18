#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
class QSerialPort;
class QLabel;
class TForm1;
class TFormConfig1;
class TForm7;
class TFormDownload;
class QCloseEvent;
class TFormDataRecord;
class TFormSerNum;
class VoltCurChart;
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void refreshPort();
    void init();
    void initConfigFile();
    void sendPortData(QByteArray data = nullptr);
    void sendSerialData(const QByteArray &data);
    void sendGetRealTimeDataCMD();
    QByteArray calculateCRCArray(const QByteArray &data, int length);
    void cacheReceiveData();
    bool receiveDataCRCCheck(const QByteArray &data);
    void dealMessage(quint8* data);
    void refreshInput();
    void refreshHolding();
    void refresh();
    QString getEventText(quint16 fault1, quint16 fault2, quint16 warn1, quint16 warn2);
    void readHoldingRegCMDBuild();
    void manualWriteOneCMDBuild(quint16 addr, quint16 value);
    void diyCMDBuild(QByteArray data, quint16 len);
    quint16 getMessageSize();
    void updateSaveDataInterval(int second);
    void startReonnect();
    void stopReconnect();
    void disconnect();
    bool ensureSaveDirectoryExists(QString serialNumberStr);
    void initializeCSVFile(QTextStream &out);
    void writeDataToCSV(QTextStream &out, const QDateTime &currentTime);
    void runTimeDeal();
    void voltCurChartInit();
private slots:
    void on_connBtn_2_clicked();
    void onSendTimerTimeout();
    void on_connBtn_clicked();
    void onReceiveTimerTimeout();
    void onTFormDestroyed(QObject *obj);
    void on_pushButton_4_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_6_clicked();
    void on_txResetTimer_timeout();
    void on_rxResetTimer_timeout();
    void on_saveDataTimer_timeout();
    void on_actionRefreshPort_triggered();
    void on_actDataRecord_triggered();
    void on_reconnectTimer_timeout();
    void on_actSerialNum_triggered();

private:
    Ui::MainWindow *ui;
    TForm1* tform1 = nullptr;
    TForm7* tform7 = nullptr;
    QSerialPort* serialPort;
    TFormConfig1* tformConfig1 = nullptr;
    TFormDownload* tformDownload = nullptr;
    TFormDataRecord* tformDataRecord = nullptr;
    TFormSerNum* tformSerNum = nullptr;
    QTimer* sendTimer = nullptr;
    QTimer* receiveTimer = nullptr;
    QTimer* saveDataTimer = nullptr;
    QTimer* reconnectTimer = nullptr;
    QLabel* connectStatusLabel;
    QLabel* versionLabel;
    QLabel* runTimeLabel;
    QTimer* txResetTimer = nullptr;
    QTimer* rxResetTimer = nullptr;
    quint8 preRunModeIndex;
    //文件写入标签
    QFile csvFile;
    VoltCurChart *voltCurChart = nullptr;
    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);
    virtual void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H
