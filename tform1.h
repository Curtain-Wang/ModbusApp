#ifndef TFORM1_H
#define TFORM1_H

#include <QWidget>

namespace Ui {
class TForm1;
}

class TForm1 : public QWidget
{
    Q_OBJECT

public:
    explicit TForm1(QWidget *parent = nullptr);
    ~TForm1();
    void refresh();
    void displayInfo(QString info);

private slots:
    void on_pushButton_2_clicked();

private:
    Ui::TForm1 *ui;
};

#endif // TFORM1_H
