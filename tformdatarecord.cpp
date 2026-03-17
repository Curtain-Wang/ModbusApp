#include "tformdatarecord.h"
#include "ui_tformdatarecord.h"
#include <QFileDialog>
#include "headfile.h"
#include <QSettings>
#include "mainwindow.h"
#include <QMessageBox>

TFormDataRecord::TFormDataRecord(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TFormDataRecord)
    , mainwindow(qobject_cast<MainWindow*>(parent))
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    init();
}

TFormDataRecord::~TFormDataRecord()
{
    delete ui;
}

void TFormDataRecord::init()
{
    if(dataRecordCycle == -1 || dataRecordFilePath == "")
    {
        //加载数据记录文件保存地址和数据记录周期
        if(!QFile::exists(CONFIG_FILE_PATH))
        {
            QString configFilePath = CONFIG_FILE_PATH;
            QSettings settings(configFilePath, QSettings::IniFormat);

            // 设置默认值
            settings.beginGroup(DATA_RECORD_CONFIG);
            settings.setValue(DATA_RECORD_FILE_PATH, QDir::currentPath());
            settings.setValue(DATA_RECORD_CYCLE, DEFAULT_DATA_RECORD_CYCLE);
            settings.endGroup();
        }
        QSettings settings(CONFIG_FILE_PATH, QSettings::IniFormat);
        settings.beginGroup(DATA_RECORD_CONFIG);
        dataRecordCycle = settings.value(DATA_RECORD_CYCLE, DEFAULT_DATA_RECORD_CYCLE).toInt();
        dataRecordFilePath = settings.value(DATA_RECORD_FILE_PATH, QDir::currentPath()).toString();
    }
    ui->lineEdit->setText(QString::number(dataRecordCycle));
    ui->lineEdit_2->setText(dataRecordFilePath);
}

void TFormDataRecord::on_pushButton_11_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(this, "选择目录", "");
    if (!directory.isEmpty()) {
        ui->lineEdit_2->setText(directory);
    }
}


void TFormDataRecord::on_pushButton_6_clicked()
{
    dataRecordCycle = ui->lineEdit->text().toInt();
    dataRecordFilePath = ui->lineEdit_2->text();
    mainwindow->updateSaveDataInterval(dataRecordCycle);
    //更新配置文件
    QSettings settings(CONFIG_FILE_PATH, QSettings::IniFormat);
    settings.beginGroup(DATA_RECORD_CONFIG);
    settings.setValue(DATA_RECORD_FILE_PATH, dataRecordFilePath);
    settings.setValue(DATA_RECORD_CYCLE, dataRecordCycle);
    settings.endGroup();
    QMessageBox::information(this, "提示", "保存成功!");
}

