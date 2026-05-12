#include "KeyRepository.h"
#include "../database/DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QDebug>

QList<ProductKey> KeyRepository::getByProductId(int productId)
{
    QList<ProductKey> result;

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT id, product_id, key_value, status, added_at, sold_at "
        "FROM product_keys WHERE product_id = :pid "
        "ORDER BY added_at DESC");
    query.bindValue(":pid", productId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "KeyRepository::getByProductId failed:" << m_lastError;
        return result;
    }

    while (query.next()) {
        ProductKey k;
        k.setId(query.value(0).toInt());
        k.setProductId(query.value(1).toInt());
        k.setKeyValue(query.value(2).toString());
        k.setStatus(ProductKey::statusFromString(query.value(3).toString()));
        k.setAddedAt(query.value(4).toDateTime());
        k.setSoldAt(query.value(5).toDateTime());
        result.append(k);
    }

    return result;
}

QList<ProductKey> KeyRepository::getAll()
{
    QList<ProductKey> result;

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT id, product_id, key_value, status, added_at, sold_at "
        "FROM product_keys ORDER BY added_at DESC");

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return result;
    }

    while (query.next()) {
        ProductKey k;
        k.setId(query.value(0).toInt());
        k.setProductId(query.value(1).toInt());
        k.setKeyValue(query.value(2).toString());
        k.setStatus(ProductKey::statusFromString(query.value(3).toString()));
        k.setAddedAt(query.value(4).toDateTime());
        k.setSoldAt(query.value(5).toDateTime());
        result.append(k);
    }

    return result;
}

ProductKey KeyRepository::getById(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT id, product_id, key_value, status, added_at, sold_at "
        "FROM product_keys WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        m_lastError = query.lastError().text();
        return ProductKey();
    }

    ProductKey k;
    k.setId(query.value(0).toInt());
    k.setProductId(query.value(1).toInt());
    k.setKeyValue(query.value(2).toString());
    k.setStatus(ProductKey::statusFromString(query.value(3).toString()));
    k.setAddedAt(query.value(4).toDateTime());
    k.setSoldAt(query.value(5).toDateTime());
    return k;
}

int KeyRepository::countAvailable(int productId)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT COUNT(*) FROM product_keys "
        "WHERE product_id = :pid AND status = 'available'");
    query.bindValue(":pid", productId);

    if (!query.exec() || !query.next()) {
        m_lastError = query.lastError().text();
        return 0;
    }

    return query.value(0).toInt();
}

bool KeyRepository::save(ProductKey& key)
{
    QSqlQuery query(DatabaseManager::instance().database());

    if (key.isNew()) {
        query.prepare("INSERT INTO product_keys (product_id, key_value, status) "
            "VALUES (:pid, :value, :status)");
    }
    else {
        query.prepare("UPDATE product_keys "
            "SET product_id = :pid, "
            "    key_value  = :value, "
            "    status     = :status, "
            "    sold_at    = :sold_at "
            "WHERE id = :id");
        query.bindValue(":id", key.id());
        query.bindValue(":sold_at", key.soldAt().isValid()
            ? QVariant(key.soldAt())
            : QVariant());
    }

    query.bindValue(":pid", key.productId());
    query.bindValue(":value", key.keyValue());
    query.bindValue(":status", ProductKey::statusToString(key.status()));

    if (!query.exec()) {
        const QString rawError = query.lastError().text();
        // SQLite кидает ошибку с упоминанием UNIQUE constraint — переведём её.
        if (rawError.contains("UNIQUE", Qt::CaseInsensitive)
            && rawError.contains("key_value", Qt::CaseInsensitive)) {
            m_lastError = "Такой ключ уже существует в базе. "
                "Каждый ключ должен быть уникальным.";
        }
        else {
            m_lastError = rawError;
        }
        qWarning() << "KeyRepository::save failed:" << m_lastError;
        return false;
    }

    if (key.isNew()) {
        key.setId(query.lastInsertId().toInt());
    }

    return true;
}

bool KeyRepository::remove(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM product_keys WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

ProductKey KeyRepository::reserveOneAvailable(int productId)
{
    QSqlDatabase db = DatabaseManager::instance().database();

    // Используем транзакцию: либо обе операции (выбор + резерв) проходят,
    // либо обе откатываются. Это защищает от повторной выдачи одного
    // и того же ключа двум параллельным заказам.
    if (!db.transaction()) {
        m_lastError = "Failed to begin transaction";
        return ProductKey();
    }

    QSqlQuery select(db);
    select.prepare("SELECT id, product_id, key_value, status, added_at "
        "FROM product_keys "
        "WHERE product_id = :pid AND status = 'available' "
        "ORDER BY added_at ASC LIMIT 1");
    select.bindValue(":pid", productId);

    if (!select.exec() || !select.next()) {
        db.rollback();
        m_lastError = "No available keys";
        return ProductKey();
    }

    ProductKey k;
    k.setId(select.value(0).toInt());
    k.setProductId(select.value(1).toInt());
    k.setKeyValue(select.value(2).toString());
    k.setStatus(ProductKey::Status::Reserved);
    k.setAddedAt(select.value(4).toDateTime());

    QSqlQuery update(db);
    update.prepare("UPDATE product_keys SET status = 'reserved' "
        "WHERE id = :id AND status = 'available'");
    update.bindValue(":id", k.id());

    if (!update.exec() || update.numRowsAffected() != 1) {
        db.rollback();
        m_lastError = "Failed to reserve key";
        return ProductKey();
    }

    if (!db.commit()) {
        db.rollback();
        m_lastError = "Failed to commit transaction";
        return ProductKey();
    }

    return k;
}