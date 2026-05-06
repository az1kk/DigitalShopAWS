#include "User.h"

User::User(int id, const QString& login,
    const QString& passwordHash, const QString& role)
    : m_id(id)
    , m_login(login)
    , m_passwordHash(passwordHash)
    , m_role(role)
{
}