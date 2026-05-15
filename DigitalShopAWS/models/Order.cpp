#include "Order.h"

Order::Order(int id, int customerId, int productId, int keyId,
    double price, Status status,
    const QDateTime& createdAt)
    : m_id(id)
    , m_customerId(customerId)
    , m_productId(productId)
    , m_keyId(keyId)
    , m_price(price)
    , m_status(status)
    , m_createdAt(createdAt)
{
}

QString Order::statusToString(Status status)
{
    switch (status) {
    case Status::Pending:   return "pending";
    case Status::Paid:      return "paid";
    case Status::Cancelled: return "cancelled";
    }
    return "pending";
}

Order::Status Order::statusFromString(const QString& str)
{
    if (str == "paid")      return Status::Paid;
    if (str == "cancelled") return Status::Cancelled;
    return Status::Pending;
}