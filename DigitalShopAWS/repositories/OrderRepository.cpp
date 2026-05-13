#include "OrderRepository.h"
#include "../database/DatabaseManager.h"
#include "../models/ProductKey.h"
#include "../models/Product.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QDebug>

QList<Order> OrderRepository::getAll()
{
    QList<Order> result;

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT id, customer_id, product_id, key_id, price, status, "
        "       created_at, paid_at "
        "FROM orders ORDER BY created_at DESC");

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "OrderRepository::getAll failed:" << m_lastError;
        return result;
    }

    while (query.next()) {
        result.append(rowToOrder(query));
    }

    return result;
}

Order OrderRepository::getById(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT id, customer_id, product_id, key_id, price, status, "
        "       created_at, paid_at "
        "FROM orders WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        m_lastError = query.lastError().text();
        return Order();
    }

    return rowToOrder(query);
}

Order OrderRepository::rowToOrder(QSqlQuery& q)
{
    Order o;
    o.setId(q.value(0).toInt());
    o.setCustomerId(q.value(1).toInt());
    o.setProductId(q.value(2).toInt());
    o.setKeyId(q.value(3).toInt());
    o.setPrice(q.value(4).toDouble());
    o.setStatus(Order::statusFromString(q.value(5).toString()));
    o.setCreatedAt(q.value(6).toDateTime());
    o.setPaidAt(q.value(7).toDateTime());
    return o;
}

Order OrderRepository::createOrder(int customerId, int productId)
{
    QSqlDatabase db = DatabaseManager::instance().database();

    if (!db.transaction()) {
        m_lastError = QString::fromUtf8("Не удалось начать транзакцию");
        return Order();
    }

    // === Шаг 1: получаем цену товара ===
    QSqlQuery priceQuery(db);
    priceQuery.prepare("SELECT price FROM products WHERE id = :id");
    priceQuery.bindValue(":id", productId);
    if (!priceQuery.exec() || !priceQuery.next()) {
        db.rollback();
        m_lastError = QString::fromUtf8("Товар не найден");
        return Order();
    }
    const double price = priceQuery.value(0).toDouble();

    // === Шаг 2: резервируем один свободный ключ ===
    QSqlQuery keySelect(db);
    keySelect.prepare("SELECT id FROM product_keys "
        "WHERE product_id = :pid AND status = 'available' "
        "ORDER BY added_at ASC LIMIT 1");
    keySelect.bindValue(":pid", productId);
    if (!keySelect.exec() || !keySelect.next()) {
        db.rollback();
        m_lastError = QString::fromUtf8("Нет доступных ключей для этого товара");
        return Order();
    }
    const int keyId = keySelect.value(0).toInt();

    QSqlQuery keyUpdate(db);
    keyUpdate.prepare("UPDATE product_keys SET status = 'reserved' "
        "WHERE id = :id AND status = 'available'");
    keyUpdate.bindValue(":id", keyId);
    if (!keyUpdate.exec() || keyUpdate.numRowsAffected() != 1) {
        db.rollback();
        m_lastError = QString::fromUtf8("Не удалось зарезервировать ключ");
        return Order();
    }

    // === Шаг 3: создаём запись заказа ===
    QSqlQuery insert(db);
    insert.prepare("INSERT INTO orders "
        "  (customer_id, product_id, key_id, price, status) "
        "VALUES (:cid, :pid, :kid, :price, 'pending')");
    insert.bindValue(":cid", customerId);
    insert.bindValue(":pid", productId);
    insert.bindValue(":kid", keyId);
    insert.bindValue(":price", price);
    if (!insert.exec()) {
        db.rollback();
        m_lastError = QString::fromUtf8("Не удалось создать заказ: ")
            + insert.lastError().text();
        return Order();
    }

    const int newId = insert.lastInsertId().toInt();

    if (!db.commit()) {
        db.rollback();
        m_lastError = QString::fromUtf8("Не удалось зафиксировать транзакцию");
        return Order();
    }

    Order o(newId, customerId, productId, keyId, price,
        Order::Status::Pending, QDateTime::currentDateTime());
    return o;
}

bool OrderRepository::confirmPayment(int orderId)
{
    QSqlDatabase db = DatabaseManager::instance().database();

    if (!db.transaction()) {
        m_lastError = QString::fromUtf8("Не удалось начать транзакцию");
        return false;
    }

    // Получаем заказ и проверяем статус.
    QSqlQuery sel(db);
    sel.prepare("SELECT key_id, status FROM orders WHERE id = :id");
    sel.bindValue(":id", orderId);
    if (!sel.exec() || !sel.next()) {
        db.rollback();
        m_lastError = QString::fromUtf8("Заказ не найден");
        return false;
    }
    const int keyId = sel.value(0).toInt();
    const QString status = sel.value(1).toString();

    if (status != "pending") {
        db.rollback();
        m_lastError = QString::fromUtf8(
            "Подтвердить оплату можно только для заказов в статусе 'ожидает оплаты'");
        return false;
    }

    // Обновляем заказ.
    QSqlQuery upd(db);
    upd.prepare("UPDATE orders "
        "SET status = 'paid', paid_at = CURRENT_TIMESTAMP "
        "WHERE id = :id");
    upd.bindValue(":id", orderId);
    if (!upd.exec()) {
        db.rollback();
        m_lastError = QString::fromUtf8("Не удалось обновить заказ");
        return false;
    }

    // Обновляем ключ — он становится проданным.
    QSqlQuery keyUpd(db);
    keyUpd.prepare("UPDATE product_keys "
        "SET status = 'sold', sold_at = CURRENT_TIMESTAMP "
        "WHERE id = :id");
    keyUpd.bindValue(":id", keyId);
    if (!keyUpd.exec()) {
        db.rollback();
        m_lastError = QString::fromUtf8("Не удалось обновить ключ");
        return false;
    }

    if (!db.commit()) {
        db.rollback();
        m_lastError = QString::fromUtf8("Не удалось зафиксировать транзакцию");
        return false;
    }

    return true;
}

bool OrderRepository::cancelOrder(int orderId)
{
    QSqlDatabase db = DatabaseManager::instance().database();

    if (!db.transaction()) {
        m_lastError = QString::fromUtf8("Не удалось начать транзакцию");
        return false;
    }

    QSqlQuery sel(db);
    sel.prepare("SELECT key_id, status FROM orders WHERE id = :id");
    sel.bindValue(":id", orderId);
    if (!sel.exec() || !sel.next()) {
        db.rollback();
        m_lastError = QString::fromUtf8("Заказ не найден");
        return false;
    }
    const int keyId = sel.value(0).toInt();
    const QString status = sel.value(1).toString();

    if (status == "cancelled") {
        db.rollback();
        return true;  // уже отменён, ошибки нет
    }

    // Возвращаем ключ в свободные.
    QSqlQuery keyUpd(db);
    keyUpd.prepare("UPDATE product_keys "
        "SET status = 'available', sold_at = NULL "
        "WHERE id = :id");
    keyUpd.bindValue(":id", keyId);
    if (!keyUpd.exec()) {
        db.rollback();
        m_lastError = QString::fromUtf8("Не удалось вернуть ключ в свободные");
        return false;
    }

    // Помечаем заказ как отменённый.
    QSqlQuery upd(db);
    upd.prepare("UPDATE orders SET status = 'cancelled' WHERE id = :id");
    upd.bindValue(":id", orderId);
    if (!upd.exec()) {
        db.rollback();
        m_lastError = QString::fromUtf8("Не удалось обновить заказ");
        return false;
    }

    if (!db.commit()) {
        db.rollback();
        m_lastError = QString::fromUtf8("Не удалось зафиксировать транзакцию");
        return false;
    }

    return true;
}