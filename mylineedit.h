#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QLineEdit>
#include <QMouseEvent>
class TForm7;
class MyLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit MyLineEdit(QWidget *parent = nullptr);
    void onTFormDestroyed(QObject *obj);

protected:
    // 重写鼠标点击事件
    void mousePressEvent(QMouseEvent *event) override;

signals:
    // 自定义点击信号
    void clicked();
private:
    TForm7* tform7 = nullptr;
};

#endif // MYLINEEDIT_H
