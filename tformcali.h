#ifndef TFORMCALI_H
#define TFORMCALI_H

#include <QWidget>
class MainWindow;

namespace Ui {
class TFormCali;
}

class TFormCali : public QWidget
{
    Q_OBJECT

public:
    explicit TFormCali(QWidget *parent = nullptr);
    ~TFormCali();
    void refresh();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

private:
    Ui::TFormCali *ui;
    MainWindow* mainwindow;
};

#endif // TFORMCALI_H
