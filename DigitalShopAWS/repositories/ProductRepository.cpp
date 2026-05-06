#include "ProductRepository.h"
#include "../database/DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

QList<Product> ProductRepository::getAll()
{
    QList<Product> result;

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT id, category_id, name, description, price "
        "FROM products ORDER BY name");

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "ProductRepository::getAll failed:" << m_lastError;
        return result;
    }

    while (query.next()) {
        Product p;
        p.setId(query.value(0).toInt());
        p.setCategoryId(query.value(1).toInt());
        p.setName(query.value(2).toString());
        p.setDescription(query.value(3).toString());
        p.setPrice(query.value(4).toDouble());
        result.append(p);
    }

    return result;
}

Product ProductRepository::getById(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT id, category_id, name, description, price "
        "FROM products WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        m_lastError = query.lastError().text();
        return Product();
    }

    Product p;
    p.setId(query.value(0).toInt());
    p.setCategoryId(query.value(1).toInt());
    p.setName(query.value(2).toString());
    p.setDescription(query.value(3).toString());
    p.setPrice(query.value(4).toDouble());
    return p;
}

bool ProductRepository::save(Product& product)
{
    QSqlQuery query(DatabaseManager::instance().database());

    if (product.isNew()) {
        // Создание нового товара — INSERT.
        query.prepare("INSERT INTO products (category_id, name, description, price) "
            "VALUES (:category_id, :name, :description, :price)");
    }
    else {
        // Обновление существующего — UPDATE.
        query.prepare("UPDATE products "
            "SET category_id = :category_id, "
            "    name        = :name, "
            "    description = :description, "
            "    price       = :price "
            "WHERE id = :id");
        query.bindValue(":id", product.id());
    }

    query.bindValue(":category_id", product.categoryId() > 0
        ? QVariant(product.categoryId())
        : QVariant());
    query.bindValue(":name", product.name());
    query.bindValue(":description", product.description());
    query.bindValue(":price", product.price());

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "ProductRepository::save failed:" << m_lastError;
        return false;
    }

    if (product.isNew()) {
        // Получаем присвоенный SQLite id для нового товара.
        product.setId(query.lastInsertId().toInt());
    }

    return true;
}

bool ProductRepository::remove(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM products WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "ProductRepository::remove failed:" << m_lastError;
        return false;
    }

    return true;
}