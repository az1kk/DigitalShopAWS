#pragma once

#include <QWidget>

class QTableWidget;
class QPushButton;
class QComboBox;

/**
 * @brief Вкладка управления заказами.
 *
 * Показывает все заказы в системе. Действия зависят от статуса заказа:
 *   - Pending  → "Подтвердить оплату", "Отменить"
 *   - Paid     → "Отменить"
 *   - Cancelled → нет действий
 *
 * Все операции изменения статуса проходят через OrderRepository,
 * который выполняет их в транзакциях.
 */
class OrdersTab : public QWidget
{
    Q_OBJECT

public:
    explicit OrdersTab(QWidget* parent = nullptr);

public slots:
    void refresh();

private slots:
    void onCreateClicked();
    void onConfirmClicked();
    void onCancelClicked();
    void onFilterChanged();

private:
    void setupUi();
    int selectedOrderId() const;

    QComboBox* m_statusFilter = nullptr;
    QTableWidget* m_table = nullptr;
    QPushButton* m_createBtn = nullptr;
    QPushButton* m_confirmBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;
    QPushButton* m_refreshBtn = nullptr;
};