#include "voltcurchart.h"
#include <QVBoxLayout>
#include <QDateTime>
#include <QtMath>
#include "headfile.h"

VoltCurChart::VoltCurChart(QWidget *parent) :
    QWidget(parent),
    m_timeWindow(10),
    m_lastVoltage(2.5)
{
    // 创建图表
    m_chart = new QChart();
    m_chart->setTitle("P侧实时电压/电流监控"); // 修改标题

    // >>>>>>>>>>>> 新增：设置标题字体大小 <<<<<<<<<<<
    QFont titleFont = m_chart->titleFont();
    titleFont.setPointSize(12); // 设置字体大小为16
    m_chart->setTitleFont(titleFont);
    // 设置图表的内边距，减少标题占用的空间
    m_chart->setMargins(QMargins(0, 0, 0, 0)); // 移除所有内边距
    // >>>>>>>>>>>> 结束新增 <<<<<<<<<<<

    // 关键优化1: 禁用所有动画效果
    m_chart->setAnimationOptions(QChart::NoAnimation);

    // 创建电压系列
    m_series = new QLineSeries();
    m_series->setName("电压(V)");
    m_series->setColor(Qt::red); // 设置电压为红色
    m_chart->addSeries(m_series);

    // >>>>>>>>>>>> 新增：创建电流系列 <<<<<<<<<<<
    m_currentSeries = new QLineSeries();
    m_currentSeries->setName("电流(A)");
    m_currentSeries->setColor(Qt::blue); // 设置电流为蓝色
    m_chart->addSeries(m_currentSeries);
    // >>>>>>>>>>>> 结束新增 <<<<<<<<<<<

    // 创建X轴（时间轴）
    m_axisX = new QDateTimeAxis();
    m_axisX->setFormat("mm:ss");
    m_axisX->setTitleText("时间(分:秒)");
    m_axisX->setTickCount(m_timeWindow + 1); // 精确设置刻度数量
    m_axisX->setGridLineVisible(true);
    m_axisX->setMinorGridLineVisible(false);
    m_axisX->setLabelsAngle(0);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);

    // >>>>>>>>>>>> 新增：电流系列也使用相同X轴 <<<<<<<<<<<
    m_currentSeries->attachAxis(m_axisX);
    // >>>>>>>>>>>> 结束新增 <<<<<<<<<<<

    // 创建Y轴（电压轴）
    m_axisY = new QValueAxis();
    m_axisY->setTitleText("电压 (V)");
    m_axisY->setLabelFormat("%.2f V");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);

    // >>>>>>>>>>>> 新增：创建电流Y轴 <<<<<<<<<<<
    m_axisCurrent = new QValueAxis();
    m_axisCurrent->setTitleText("电流 (A)");
    m_axisCurrent->setLabelFormat("%.2f A");
    m_chart->addAxis(m_axisCurrent, Qt::AlignRight);
    m_currentSeries->attachAxis(m_axisCurrent);
    // >>>>>>>>>>>> 结束新增 <<<<<<<<<<<

    // 创建ChartView
    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 设置布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_chartView);
    setLayout(layout);

    // 关键优化3: 确保时间轴初始对齐
    alignTimeAxis();

    // 关键优化4: 初始化时预填充时间窗口的数据
    prefillData();

    // 关键优化5: 确保时间轴范围正确
    forceAxisAlignment();

    // 创建定时器
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &VoltCurChart::updateChart);
}

VoltCurChart::~VoltCurChart()
{
    stop();
}

void VoltCurChart::setTimeWindow(int seconds)
{
    m_timeWindow = qBound(5, seconds, 60); // 限制在5-60秒之间
    // 更新X轴刻度数量
    m_axisX->setTickCount(m_timeWindow + 1);

    // 更新X轴范围
    alignTimeAxis();

    // 重新预填充数据
    prefillData();
    forceAxisAlignment();
}

void VoltCurChart::setVoltageRange(double min, double max)
{
    m_axisY->setMin(min);
    m_axisY->setMax(max);
}

void VoltCurChart::setCurrentRange(double min, double max)
{
    m_axisCurrent->setMin(min);
    m_axisCurrent->setMax(max);
}

void VoltCurChart::addCurrentPoint(double current)
{
    // 使用对齐到秒的时间戳
    QDateTime now = alignedNow();
    qint64 timestamp = now.toMSecsSinceEpoch();

    // 如果已经有相同时间戳的数据，替换它
    bool found = false;
    for (int i = 0; i < m_currentSeries->count(); i++) {
        if (qFuzzyCompare(m_currentSeries->at(i).x(), timestamp)) {
            m_currentSeries->replace(i, timestamp, current);
            found = true;
            break;
        }
    }

    // 如果没有找到相同时间戳的数据，添加新点
    if (!found) {
        m_currentSeries->append(timestamp, current);
    }

    // 确保时间轴始终对齐
    alignTimeAxis();

    // 移除过期数据
    qint64 cutoffTime = now.addSecs(-m_timeWindow).toMSecsSinceEpoch();
    int pointsToRemove = 0;

    for (int i = 0; i < m_currentSeries->count(); i++) {
        if (m_currentSeries->at(i).x() < cutoffTime) {
            pointsToRemove++;
        } else {
            break;
        }
    }

    if (pointsToRemove > 0) {
        m_currentSeries->removePoints(0, pointsToRemove);
    }

    // 确保时间轴范围正确
    forceAxisAlignment();
}

QDateTime VoltCurChart::alignedNow() const
{
    QDateTime now = QDateTime::currentDateTime();
    // 将毫秒设置为0，确保时间戳精确到秒
    return now.addMSecs(-now.time().msec());
}

void VoltCurChart::alignTimeAxis()
{
    QDateTime now = alignedNow();
    m_axisX->setMin(now.addSecs(-m_timeWindow));
    m_axisX->setMax(now);
}

void VoltCurChart::prefillData()
{
    // 清除现有数据
    m_series->clear();
    m_currentSeries->clear();

    // 从时间窗口的起点开始填充
    QDateTime startTime = alignedNow().addSecs(-m_timeWindow);

    // 预填充m_timeWindow秒的数据
    for (int i = 0; i <= m_timeWindow; i++) {
        QDateTime timestamp = startTime.addSecs(i);
        m_series->append(timestamp.toMSecsSinceEpoch(), 0);
        m_currentSeries->append(timestamp.toMSecsSinceEpoch(), 0);
    }
}

void VoltCurChart::forceAxisAlignment()
{
    QDateTime now = alignedNow();
    m_axisX->setMin(now.addSecs(-m_timeWindow));
    m_axisX->setMax(now);

    // 确保图表立即更新
    m_chart->update();
}

void VoltCurChart::addVoltagePoint(double voltage)
{
    // 使用对齐到秒的时间戳
    QDateTime now = alignedNow();
    qint64 timestamp = now.toMSecsSinceEpoch();

    // 如果已经有相同时间戳的数据，替换它
    bool found = false;
    for (int i = 0; i < m_series->count(); i++) {
        if (qFuzzyCompare(m_series->at(i).x(), timestamp)) {
            m_series->replace(i, timestamp, voltage);
            found = true;
            break;
        }
    }

    // 如果没有找到相同时间戳的数据，添加新点
    if (!found) {
        m_series->append(timestamp, voltage);
    }

    // 确保时间轴始终对齐
    alignTimeAxis();

    // 移除过期数据
    qint64 cutoffTime = now.addSecs(-m_timeWindow).toMSecsSinceEpoch();
    int pointsToRemove = 0;

    for (int i = 0; i < m_series->count(); i++) {
        if (m_series->at(i).x() < cutoffTime) {
            pointsToRemove++;
        } else {
            break;
        }
    }

    if (pointsToRemove > 0) {
        m_series->removePoints(0, pointsToRemove);
    }

    // 确保时间轴范围正确
    forceAxisAlignment();
    // 动态调整Y轴范围
    // double minY = voltage, maxY = voltage;
    // for (int i = 0; i < m_series->count(); i++) {
    //     double y = m_series->at(i).y();
    //     minY = qMin(minY, y);
    //     maxY = qMax(maxY, y);
    // }
    // m_axisY->setRange(0, maxY * 1.05);
}

void VoltCurChart::start()
{
    if (!m_timer->isActive()) {
        m_timer->start();
    }
}

void VoltCurChart::stop()
{
    if (m_timer->isActive()) {
        m_timer->stop();
    }
}

void VoltCurChart::swtichB()
{
    flag = 1;
    m_chart->setTitle("B侧实时电压/电流监控"); // 修改标题
}

void VoltCurChart::updateChart()
{
    if(connFlag != CONNECTED)
    {
        m_timer->stop();
    }

    m_timer->setSingleShot(true);

    if(flag == 0)
    {
        // 添加新数据点
        addVoltagePoint(g_TelRegs[2] / 100.0);
        addCurrentPoint(static_cast<qint16>(g_TelRegs[3]) / 100.0);
    }else
    {
        addVoltagePoint(g_TelRegs[0] / 100.0);
        addCurrentPoint(static_cast<qint16>(g_TelRegs[1]) / 100.0);
    }


    m_timer->setSingleShot(false);


}
