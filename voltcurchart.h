#ifndef VOLTCURCHART_H
#define VOLTCURCHART_H

#include <QWidget>
#include <QtCharts>
#include <QTimer>

class VoltCurChart : public QWidget
{
    Q_OBJECT
public:
    explicit VoltCurChart(QWidget *parent = nullptr);
    ~VoltCurChart();

    // 设置显示的时间窗口（秒）
    void setTimeWindow(int seconds = 20);

    // 设置Y轴范围（电压）
    void setVoltageRange(double min = 0, double max = 5);

    // 设置右侧Y轴范围（电流）
    void setCurrentRange(double min = 0, double max = 1);

    // 添加新的电压数据点
    void addVoltagePoint(double voltage);

    // 添加新的电流数据点
    void addCurrentPoint(double current);

    // 开始/停止数据更新
    void start();
    void stop();
    void swtichB();

private slots:
    void updateChart();

private:
    // 获取精确对齐到秒的时间戳
    QDateTime alignedNow() const;

    // 确保时间轴范围对齐到整秒
    void alignTimeAxis();

    // 预填充初始数据
    void prefillData();

    // 强制时间轴范围对齐
    void forceAxisAlignment();

    QChart *m_chart;
    QChartView *m_chartView;
    QLineSeries *m_series;        // 电压系列
    QLineSeries *m_currentSeries; // 电流系列（新增）
    QDateTimeAxis *m_axisX;
    QValueAxis *m_axisY;          // 电压Y轴
    QValueAxis *m_axisCurrent;    // 电流Y轴（新增）

    QTimer *m_timer;
    int m_timeWindow; // 时间窗口(秒)

    // 用于模拟数据的内部变量
    double m_lastVoltage;
    double m_lastCurrent; // 电流值（新增）
    int flag = 0;   //默认P侧
};

#endif // VOLTCURCHART_H
