#pragma once

#include <QWidget>
#include <QVector>

#include "../opengl/SalesChart3D.h"   // отсюда берём struct ChartBar

class QVBoxLayout;

/**
 * @brief Вкладка аналитики.
 *
 * Содержит 3D-диаграмму продаж (OpenGL) и легенду с подписями.
 */
class AnalyticsTab : public QWidget
{
    Q_OBJECT

public:
    explicit AnalyticsTab(QWidget* parent = nullptr);

public slots:
    void refresh();

private:
    void setupUi();
    void rebuildLegend(const QVector<ChartBar>& bars);

    SalesChart3D* m_chart = nullptr;
    QVBoxLayout* m_legendLayout = nullptr;
    QWidget* m_legendContainer = nullptr;
};