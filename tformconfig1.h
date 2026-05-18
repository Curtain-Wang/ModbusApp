#ifndef TFORMCONFIG1_H
#define TFORMCONFIG1_H

#include <QWidget>
class MainWindow;

namespace Ui {
class TFormConfig1;
}

class TFormConfig1 : public QWidget
{
    Q_OBJECT

public:
    explicit TFormConfig1(QWidget *parent = nullptr);
    ~TFormConfig1();
    void init();
    void refresh();
private slots:

private:
    Ui::TFormConfig1 *ui;
    MainWindow* mainwindow;
    quint8 preRunModeIndex;
    quint8 preDimingModeIndex;
    quint8 preFanCtrlModeIndex;
    quint8 preStartModeIndex;
};

#endif // TFORMCONFIG1_H
