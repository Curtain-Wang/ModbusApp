#ifndef TFORMBATCH_H
#define TFORMBATCH_H

#include <QWidget>

namespace Ui {
class TFormBatch;
}

class TFormBatch : public QWidget
{
    Q_OBJECT

public:
    explicit TFormBatch(QWidget *parent = nullptr);
    ~TFormBatch();

private slots:
    void on_checkBox_9_clicked(bool checked);

private:
    Ui::TFormBatch *ui;
};

#endif // TFORMBATCH_H
