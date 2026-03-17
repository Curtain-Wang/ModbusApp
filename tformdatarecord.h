#ifndef TFORMDATARECORD_H
#define TFORMDATARECORD_H

#include <QWidget>
class MainWindow;

namespace Ui {
class TFormDataRecord;
}

class TFormDataRecord : public QWidget
{
    Q_OBJECT

public:
    explicit TFormDataRecord(QWidget *parent = nullptr);
    ~TFormDataRecord();
    void init();
private slots:
    void on_pushButton_11_clicked();

    void on_pushButton_6_clicked();

private:
    Ui::TFormDataRecord *ui;
    MainWindow* mainwindow;
};

#endif // TFORMDATARECORD_H
