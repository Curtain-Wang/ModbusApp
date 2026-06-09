#ifndef TFORMSERNUM_H
#define TFORMSERNUM_H

#include <QWidget>
class MainWindow;
namespace Ui {
class TFormSerNum;
}

class TFormSerNum : public QWidget
{
    Q_OBJECT

public:
    explicit TFormSerNum(QWidget *parent = nullptr);
    ~TFormSerNum();
private slots:
    void on_pushButton_7_clicked();

private:
    Ui::TFormSerNum *ui;
    MainWindow* mainwindow;
};

#endif // TFORMSERNUM_H
