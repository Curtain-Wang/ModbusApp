#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class QSerialPort;
class QLabel;
class TForm1;
class TFormConfig1;
class TForm7;
class TFormDownload;

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
    void regPowInit();
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
    QString getEventText(quint16 value);
    void readHoldingRegCMDBuild();
    void manualWriteOneCMDBuild(quint16 addr, quint16 value);
    void diyCMDBuild(QByteArray data, quint16 len);
    quint16 getMessageSize();
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

private:
    Ui::MainWindow *ui;
    TForm1* tform1 = nullptr;
    TForm7* tform7 = nullptr;
    QSerialPort* serialPort;
    TFormConfig1* tformConfig1 = nullptr;
    TFormDownload* tformDownload = nullptr;
    QTimer* sendTimer = nullptr;
    QTimer* receiveTimer = nullptr;
    QLabel* connectStatusLabel;
    QLabel* versionLabel;
    QTimer* txResetTimer = nullptr;
    QTimer* rxResetTimer = nullptr;
};
#endif // MAINWINDOW_H
