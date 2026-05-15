#include "AnalyticsTab.h"
#include "../opengl/SalesChart3D.h"
#include "../repositories/OrderRepository.h"
#include "../models/Order.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QMap>
#include <QDate>
#include <QDateTime>

AnalyticsTab::AnalyticsTab(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    refresh();
}

void AnalyticsTab::setupUi()
{
    m_chart = new SalesChart3D(this);

    // Контейнер для легенды (подписи столбцов с цветами).
    m_legendContainer = new QWidget(this);
    m_legendLayout = new QVBoxLayout(m_legendContainer);
    m_legendLayout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* legendBox = new QGroupBox(QString::fromUtf8("Легенда"));
    QVBoxLayout* legendBoxLayout = new QVBoxLayout(legendBox);
    legendBoxLayout->addWidget(m_legendContainer);
    legendBoxLayout->addStretch();
    legendBox->setFixedWidth(280);

    QPushButton* refreshBtn = new QPushButton(QString::fromUtf8("Обновить"), this);
    connect(refreshBtn, &QPushButton::clicked, this, &AnalyticsTab::refresh);

    // Диаграмма слева, легенда справа.
    QHBoxLayout* content = new QHBoxLayout;
    content->addWidget(m_chart, 1);
    content->addWidget(legendBox);

    QHBoxLayout* buttonRow = new QHBoxLayout;
    buttonRow->addWidget(new QLabel(
        QString::fromUtf8("<i>Зажмите и тяните мышью, чтобы вращать диаграмму</i>")));
    buttonRow->addStretch();
    buttonRow->addWidget(refreshBtn);

    QVBoxLayout* main = new QVBoxLayout(this);
    main->addWidget(new QLabel(
        QString::fromUtf8("<b>Аналитика продаж</b>")));
    main->addLayout(content, 1);
    main->addLayout(buttonRow);
}

void AnalyticsTab::rebuildLegend(const QVector<ChartBar>& bars)
{
    // Удаляем старые элементы легенды.
    QLayoutItem* item;
    while ((item = m_legendLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Строим заново: цветной квадрат + подпись + значение.
    for (int i = 0; i < bars.size(); ++i) {
        const ChartBar& bar = bars[i];

        QWidget* row = new QWidget(m_legendContainer);
        QHBoxLayout* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 2, 0, 2);

        QLabel* colorBox = new QLabel(row);
        colorBox->setFixedSize(16, 16);
        const QColor c = SalesChart3D::barColor(i);
        colorBox->setStyleSheet(
            QString("background-color: %1; border: 1px solid #888;")
            .arg(c.name()));

        QLabel* text = new QLabel(
            QString("%1 — %2")
            .arg(bar.label)
            .arg(QString::number(bar.value, 'f', 0)),
            row);

        rowLayout->addWidget(colorBox);
        rowLayout->addWidget(text, 1);

        m_legendLayout->addWidget(row);
    }
}

void AnalyticsTab::refresh()
{
    // Берём оплаченные заказы и агрегируем выручку по месяцам.
    OrderRepository orderRepo;
    const QList<Order> all = orderRepo.getAll();

    // Ключ карты — "YYYY-MM", значение — сумма выручки за этот месяц.
    QMap<QString, double> revenueByMonth;

    for (const Order& o : all) {
        if (o.status() != Order::Status::Paid) {
            continue;
        }
        // У оплаченного заказа берём дату оплаты, если есть; иначе — дату создания.
        const QDateTime dt = o.paidAt().isValid() ? o.paidAt() : o.createdAt();
        if (!dt.isValid()) {
            continue;
        }
        const QString key = dt.toString("yyyy-MM");
        revenueByMonth[key] += o.price();
    }

    // Строим список из 6 последних месяцев, начиная с того, что 5 месяцев назад,
    // и заканчивая текущим. В каждом — либо реальная выручка, либо 0.
    const QDate today = QDate::currentDate();
    QVector<ChartBar> bars;

    static const QStringList monthNames = {
        QString::fromUtf8("Янв"), QString::fromUtf8("Фев"), QString::fromUtf8("Мар"),
        QString::fromUtf8("Апр"), QString::fromUtf8("Май"), QString::fromUtf8("Июн"),
        QString::fromUtf8("Июл"), QString::fromUtf8("Авг"), QString::fromUtf8("Сен"),
        QString::fromUtf8("Окт"), QString::fromUtf8("Ноя"), QString::fromUtf8("Дек")
    };

    for (int i = 5; i >= 0; --i) {
        const QDate d = today.addMonths(-i);
        const QString key = d.toString("yyyy-MM");
        const double value = revenueByMonth.value(key, 0.0);

        // Подпись: "Май 26" (месяц + последние 2 цифры года).
        const QString label = QString("%1 %2")
            .arg(monthNames.value(d.month() - 1))
            .arg(d.year() % 100, 2, 10, QChar('0'));

        bars.append({ label, value });
    }

    m_chart->setData(bars);
    rebuildLegend(bars);
}