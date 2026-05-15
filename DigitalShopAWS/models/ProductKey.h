#pragma once

#include <QString>
#include <QDateTime>

/**
 * @brief Модель ключа цифрового товара.
 *
 * Один объект Product может иметь много ключей (склад на отдельные ключи).
 * Каждый ключ имеет статус, по которому ясно: можно ли его выдать клиенту.
 *
 * Связан с таблицей product_keys в БД.
 */
class ProductKey
{
public:
    /// Возможные статусы ключа.
    enum class Status {
        Available,  // свободен, может быть выдан
        Sold,       // уже продан клиенту
        Reserved    // зарезервирован под заказ, ждёт оплаты
    };

    ProductKey() = default;
    ProductKey(int id, int productId, const QString& keyValue,
        Status status, const QDateTime& addedAt = QDateTime::currentDateTime());

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    int productId() const { return m_productId; }
    void setProductId(int productId) { m_productId = productId; }

    QString keyValue() const { return m_keyValue; }
    void setKeyValue(const QString& value) { m_keyValue = value; }

    Status status() const { return m_status; }
    void setStatus(Status status) { m_status = status; }

    QDateTime addedAt() const { return m_addedAt; }
    void setAddedAt(const QDateTime& dt) { m_addedAt = dt; }

    QDateTime soldAt() const { return m_soldAt; }
    void setSoldAt(const QDateTime& dt) { m_soldAt = dt; }

    bool isNew() const { return m_id == 0; }
    bool isAvailable() const { return m_status == Status::Available; }

    /// Конвертация статуса в строку для БД.
    static QString statusToString(Status status);
    /// Обратная конвертация.
    static Status statusFromString(const QString& str);

private:
    int       m_id = 0;
    int       m_productId = 0;
    QString   m_keyValue;
    Status    m_status = Status::Available;
    QDateTime m_addedAt;
    QDateTime m_soldAt;
};
