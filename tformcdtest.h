#ifndef TFORMCDTEST_H
#define TFORMCDTEST_H

#include <QWidget>
class QTimer;
namespace Ui {
class TFormCDTest;
}

class TFormCDTest : public QWidget
{
    Q_OBJECT

public:
    explicit TFormCDTest(QWidget *parent = nullptr);
    ~TFormCDTest();

private slots:
    void on_timer0_timeout();

    void on_pushButton_clicked();

private:
    Ui::TFormCDTest *ui;
    QTimer* timer0 = nullptr;
    quint16 count;
    quint16 cycleCount;
};

#endif // TFORMCDTEST_H
