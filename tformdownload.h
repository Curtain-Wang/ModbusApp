#ifndef TFORMDOWNLOAD_H
#define TFORMDOWNLOAD_H
#define OUTPUT_SWID     0x0101
#define INPUT_SWID      0x0202
#define MASTER 0x01
#define SLAVE 0x02

#include <QWidget>
class QTimer;
namespace Ui {
class TFormDownload;
}

class TFormDownload : public QWidget
{
    Q_OBJECT

public:
    explicit TFormDownload(QWidget *parent = nullptr);
    ~TFormDownload();
    void init();
    void endDownload();
    void sendData();
    void downloadRespDeal();
    QString getInitDir();
    void updateInitDir(QString dir);
private slots:
    void on_timeout();

    void on_pushButton_clicked();

    void on_master_btn_clicked();

private:
    Ui::TFormDownload *ui;
    QTimer* timer;
    quint16 swId;
    quint16 downVer;
    bool confirmed = false;
    //升级标志,1, 2从机
    quint8 upgradeFlag;
    quint16 pageAddr;
    quint16 DownloadTime;
    quint8 DownloadRepeatNum;
    quint8 DownloadTXFlag;
    quint32 fileLen;
    QByteArray fileBuf;
    quint16 fileCRC;
    int downloadlen;
    bool reDownloadFlag;

};

#endif // TFORMDOWNLOAD_H
