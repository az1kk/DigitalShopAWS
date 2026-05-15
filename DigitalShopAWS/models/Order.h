#pragma once

#include <QString>
#include <QDateTime>

/**
 * @brief Модель заказа.
 *
 * Заказ — это запись о покупке: клиент, товар, конкретный ключ, статус.
 * Жизненный цикл заказа:
 *   Pending   — создан, ждёт оплаты. Ключ зарезервирован.
 *   Paid      — оплата подтверждена. Ключ помечен как проданный.
 *   Cancelled — заказ отменён. Ключ возвращён в свободные.
 *
 * Связан с таблицей orders в БД.
 */
class Order
{
public:
    enum class Status {
        Pending,    // ожидает оплаты
        Paid,       // оплачен, ключ выдан
        Cancelled   // отменён
    };

    Order() = default;
    Order(int id, int customerId, int productId, int keyId,
        double price, Status status,
        const QDateTime& createdAt = QDateTime::currentDateTime());

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    int customerId() const { return m_customerId; }
    void setCustomerId(int customerId) { m_customerId = customerId; }

    int productId() const { return m_productId; }
    void setProductId(int productId) { m_productId = productId; }

    int keyId() const { return m_keyId; }
    void setKeyId(int keyId) { m_keyId = keyId; }

    double price() const { return m_price; }
    void setPrice(double price) { m_price = price; }

    Status status() const { return m_status; }
    void setStatus(Status status) { m_status = status; }

    QDateTime createdAt() const { return m_createdAt; }
    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }

    QDateTime paidAt() const { return m_paidAt; }
    void setPaidAt(const QDateTime& dt) { m_paidAt = dt; }

    bool isNew() const { return m_id == 0; }

    static QString statusToString(Status status);
    static Status  statusFromString(const QString& str);

private:
    int       m_id = 0;
    int       m_customerId = 0;
    int       m_productId = 0;
    int       m_keyId = 0;
    double    m_price = 0.0;
    Status    m_status = Status::Pending;
    QDateTime m_createdAt;
    QDateTime m_paidAt;
};