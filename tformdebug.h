#ifndef TFORMDEBUG_H
#define TFORMDEBUG_H

#include <QWidget>
class MainWindow;
namespace Ui {
class TFormDebug;
}

class TFormDebug : public QWidget
{
    Q_OBJECT

public:
    explicit TFormDebug(QWidget *parent = nullptr);
    ~TFormDebug();
    void init();
    void refresh();

private:
    Ui::TFormDebug *ui;
    MainWindow* mainwindow;
};

#endif // TFORMDEBUG_H
