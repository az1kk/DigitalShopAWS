#pragma once

#include <QWidget>

class QLabel;
class QPushButton;

/**
 * @brief Вкладка сводной статистики системы.
 *
 * Показывает счётчики по товарам, клиентам, ключам, заказам
 * и общую выручку. Данные собираются из готовых репозиториев.
 */
class StatisticsTab : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsTab(QWidget* parent = nullptr);

public slots:
    /// Пересчитывает все показатели и обновляет надписи.
    void refresh();

private:
    void setupUi();

    // Каталог
    QLabel* m_productsLabel = nullptr;
    QLabel* m_customersLabel = nullptr;

    // Ключи
    QLabel* m_keysTotalLabel = nullptr;
    QLabel* m_keysAvailableLabel = nullptr;
    QLabel* m_keysReservedLabel = nullptr;
    QLabel* m_keysSoldLabel = nullptr;

    // Заказы
    QLabel* m_ordersTotalLabel = nullptr;
    QLabel* m_ordersPendingLabel = nullptr;
    QLabel* m_ordersPaidLabel = nullptr;
    QLabel* m_ordersCancelledLabel = nullptr;

    // Финансы
    QLabel* m_revenueLabel = nullptr;

    QPushButton* m_refreshBtn = nullptr;
};