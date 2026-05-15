#pragma once

#include <QString>

/**
 * @brief Модель клиента.
 * Связан с таблицей customers в БД.
 */
class Customer
{
public:
    Customer() = default;
    Customer(int id, const QString& name,
        const QString& email, const QString& phone);

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    QString email() const { return m_email; }
    void setEmail(const QString& email) { m_email = email; }

    QString phone() const { return m_phone; }
    void setPhone(const QString& phone) { m_phone = phone; }

    bool isNew() const { return m_id == 0; }

private:
    int     m_id = 0;
    QString m_name;
    QString m_email;
    QString m_phone;
};
