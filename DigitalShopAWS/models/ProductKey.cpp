#include "ProductKey.h"

ProductKey::ProductKey(int id, int productId, const QString& keyValue,
    Status status, const QDateTime& addedAt)
    : m_id(id)
    , m_productId(productId)
    , m_keyValue(keyValue)
    , m_status(status)
    , m_addedAt(addedAt)
{
}

QString ProductKey::statusToString(Status status)
{
    switch (status) {
    case Status::Available: return "available";
    case Status::Sold:      return "sold";
    case Status::Reserved:  return "reserved";
    }
    return "available";
}

ProductKey::Status ProductKey::statusFromString(const QString& str)
{
    if (str == "sold")     return Status::Sold;
    if (str == "reserved") return Status::Reserved;
    return Status::Available;
}