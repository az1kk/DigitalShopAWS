#include "CustomerRepository.h"
#include "../database/DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

QList<Customer> CustomerRepository::getAll()
{
    QList<Customer> result;

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT id, name, email, phone "
        "FROM customers ORDER BY name");

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "CustomerRepository::getAll failed:" << m_lastError;
        return result;
    }

    while (query.next()) {
        Customer c;
        c.setId(query.value(0).toInt());
        c.setName(query.value(1).toString());
        c.setEmail(query.value(2).toString());
        c.setPhone(query.value(3).toString());
        result.append(c);
    }

    return result;
}

Customer CustomerRepository::getById(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT id, name, email, phone "
        "FROM customers WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        m_lastError = query.lastError().text();
        return Customer();
    }

    Customer c;
    c.setId(query.value(0).toInt());
    c.setName(query.value(1).toString());
    c.setEmail(query.value(2).toString());
    c.setPhone(query.value(3).toString());
    return c;
}

bool CustomerRepository::save(Customer& customer)
{
    QSqlQuery query(DatabaseManager::instance().database());

    if (customer.isNew()) {
        query.prepare("INSERT INTO customers (name, email, phone) "
            "VALUES (:name, :email, :phone)");
    }
    else {
        query.prepare("UPDATE customers "
            "SET name = :name, "
            "    email = :email, "
            "    phone = :phone "
            "WHERE id = :id");
        query.bindValue(":id", customer.id());
    }

    query.bindValue(":name", customer.name());
    query.bindValue(":email", customer.email().isEmpty()
        ? QVariant() : QVariant(customer.email()));
    query.bindValue(":phone", customer.phone());

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "CustomerRepository::save failed:" << m_lastError;
        return false;
    }

    if (customer.isNew()) {
        customer.setId(query.lastInsertId().toInt());
    }

    return true;
}

bool CustomerRepository::remove(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM customers WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "CustomerRepository::remove failed:" << m_lastError;
        return false;
    }

    return true;
}