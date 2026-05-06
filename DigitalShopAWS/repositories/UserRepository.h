#pragma once

#include "../models/User.h"
#include <QString>

/**
 * @brief Репозиторий пользователей.
 *
 * Главный сценарий — найти пользователя по логину для проверки пароля
 * при входе в систему.
 */
class UserRepository
{
public:
    /// Находит пользователя по логину.
    /// Возвращает User() с id=0, если не найден.
    User getByLogin(const QString& login);

    /// Сохраняет пользователя (INSERT при isNew(), иначе UPDATE).
    bool save(User& user);

    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;
};