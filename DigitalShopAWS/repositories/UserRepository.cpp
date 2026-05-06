#include "UserRepository.h"
#include "../database/DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

User UserRepository::getByLogin(const QString& login)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT id, login, password_hash, role "
        "FROM users WHERE login = :login");
    query.bindValue(":login", login);

    if (!query.exec() || !query.next()) {
        m_lastError = query.lastError().text();
        return User();
    }

    User u;
    u.setId(query.value(0).toInt());
    u.setLogin(query.value(1).toString());
    u.setPasswordHash(query.value(2).toString());
    u.setRole(query.value(3).toString());
    return u;
}

bool UserRepository::save(User& user)
{
    QSqlQuery query(DatabaseManager::instance().database());

    if (user.isNew()) {
        query.prepare("INSERT INTO users (login, password_hash, role) "
            "VALUES (:login, :password_hash, :role)");
    }
    else {
        query.prepare("UPDATE users "
            "SET login = :login, "
            "    password_hash = :password_hash, "
            "    role = :role "
            "WHERE id = :id");
        query.bindValue(":id", user.id());
    }

    query.bindValue(":login", user.login());
    query.bindValue(":password_hash", user.passwordHash());
    query.bindValue(":role", user.role());

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "UserRepository::save failed:" << m_lastError;
        return false;
    }

    if (user.isNew()) {
        user.setId(query.lastInsertId().toInt());
    }

    return true;
}