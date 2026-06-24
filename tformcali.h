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

private:
    Ui::TFormCali *ui;
    MainWindow* mainwindow;
};

#endif // TFORMCALI_H
