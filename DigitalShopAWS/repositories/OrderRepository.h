#pragma once

#include "../models/Order.h"
#include <QList>
#include <QString>

/**
 * @brief Репозиторий заказов.
 *
 * Содержит атомарные операции жизненного цикла заказа.
 * Все они работают в транзакциях, чтобы данные orders и product_keys
 * всегда оставались согласованными.
 */
class OrderRepository
{
public:
    /// Возвращает все заказы, отсортированные по дате (свежие сверху).
    QList<Order> getAll();

    /// Находит заказ по id.
    Order getById(int id);

    /**
     * @brief Создаёт новый заказ.
     *
     * Атомарно выполняет три действия:
     *   1. Резервирует один свободный ключ товара.
     *   2. Создаёт запись в orders со статусом 'pending'.
     *   3. Привязывает ключ к заказу.
     *
     * Если на любом шаге что-то пошло не так — транзакция откатывается,
     * никаких изменений в БД не остаётся.
     *
     * @param customerId — клиент.
     * @param productId  — товар.
     * @return заполненный Order при успехе; Order с id=0 при отсутствии
     *         свободных ключей или другой ошибке.
     */
    Order createOrder(int customerId, int productId);

    /**
     * @brief Подтверждает оплату заказа.
     *
     * Атомарно:
     *   1. Переводит заказ из 'pending' в 'paid', проставляет paid_at.
     *   2. Переводит привязанный ключ из 'reserved' в 'sold',
     *      проставляет sold_at.
     */
    bool confirmPayment(int orderId);

    /**
     * @brief Отменяет заказ.
     *
     * Атомарно:
     *   1. Переводит заказ в 'cancelled'.
     *   2. Возвращает привязанный ключ из 'reserved'/'sold' обратно в 'available'.
     *
     * Если заказ уже отменён — ничего не делает, возвращает true.
     */
    bool cancelOrder(int orderId);

    QString lastError() const { return m_lastError; }

private:
    /// Помощник — превращает строку SQL-результата в Order.
    Order rowToOrder(class QSqlQuery& q);

    QString m_lastError;
};