#include "AnalyticsTab.h"
#include "../opengl/SalesChart3D.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

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
    // === ВРЕМЕННЫЕ ТЕСТОВЫЕ ДАННЫЕ ===
    // На следующем шаге заменим на реальные данные из БД.
    QVector<ChartBar> bars;
    bars.append({ QString::fromUtf8("Январь"),  120000 });
    bars.append({ QString::fromUtf8("Февраль"),  85000 });
    bars.append({ QString::fromUtf8("Март"),    156000 });
    bars.append({ QString::fromUtf8("Апрель"),   98000 });
    bars.append({ QString::fromUtf8("Май"),     142000 });
    // =================================

    m_chart->setData(bars);
    rebuildLegend(bars);
}