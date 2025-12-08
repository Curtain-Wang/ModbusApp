#ifndef TFORM7_H
#define TFORM7_H

#include <QWidget>

namespace Ui {
class TForm7;
}

class TForm7 : public QWidget
{
    Q_OBJECT

public:
    explicit TForm7(QWidget *parent = nullptr);
    ~TForm7();
private slots:
    void on_lineEdit_returnPressed();

private:
    Ui::TForm7 *ui;
};

#endif // TFORM7_H
