#include "Customer.h"

Customer::Customer(int id, const QString& name,
    const QString& email, const QString& phone)
    : m_id(id)
    , m_name(name)
    , m_email(email)
    , m_phone(phone)
{
}