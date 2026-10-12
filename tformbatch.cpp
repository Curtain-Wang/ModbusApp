#include "tformbatch.h"
#include "ui_tformbatch.h"

TFormBatch::TFormBatch(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TFormBatch)
{
    //设置窗口标志，确保有边框和标题栏、最小化、关闭，最大化
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    ui->setupUi(this);

}

TFormBatch::~TFormBatch()
{
    delete ui;
}

void TFormBatch::on_checkBox_9_clicked(bool checked)
{
    QList<QCheckBox*> targetCheckBoxes = {
        ui->checkBox, ui->checkBox_2, ui->checkBox_3,
        ui->checkBox_4, ui->checkBox_5, ui->checkBox_6,
        ui->checkBox_7, ui->checkBox_8
    };
    for(QCheckBox* cb : targetCheckBoxes)
    {
        if(cb != nullptr)
        {
            cb->setChecked(checked);
        }
    }
}


void TFormBatch::on_connBtn_clicked()
{

}

