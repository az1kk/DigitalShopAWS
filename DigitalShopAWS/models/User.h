#pragma once

#include <QString>

/**
 * @brief Модель пользователя системы (для авторизации).
 * Связан с таблицей users в БД.
 */
class User
{
public:
    User() = default;
    User(int id, const QString& login,
        const QString& passwordHash, const QString& role);

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString login() const { return m_login; }
    void setLogin(const QString& login) { m_login = login; }

    QString passwordHash() const { return m_passwordHash; }
    void setPasswordHash(const QString& hash) { m_passwordHash = hash; }

    QString role() const { return m_role; }
    void setRole(const QString& role) { m_role = role; }

    bool isNew() const { return m_id == 0; }
    bool isValid() const { return m_id > 0; }

private:
    int     m_id = 0;
    QString m_login;
    QString m_passwordHash;
    QString m_role = "admin";
};