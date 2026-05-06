#include "Product.h"

Product::Product(int id, int categoryId, const QString& name,
    const QString& description, double price)
    : m_id(id)
    , m_categoryId(categoryId)
    , m_name(name)
    , m_description(description)
    , m_price(price)
{
}