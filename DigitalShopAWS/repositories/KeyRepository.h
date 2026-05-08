#pragma once

#include "../models/ProductKey.h"
#include <QList>
#include <QString>

/**
 * @brief Репозиторий ключей.
 *
 * Помимо обычных CRUD-методов содержит специальный метод reserveOneAvailable,
 * который атомарно резервирует один свободный ключ для заказа.
 * Это ключевой механизм защиты от повторной выдачи одного ключа двум клиентам.
 */
class KeyRepository
{
public:
    /// Возвращает все ключи определённого товара.
    QList<ProductKey> getByProductId(int productId);

    /// Возвращает все ключи в системе (для общей таблицы).
    QList<ProductKey> getAll();

    /// Находит ключ по id.
    ProductKey getById(int id);

    /// Считает количество свободных ключей данного товара.
    int countAvailable(int productId);

    /// Сохраняет ключ. INSERT при isNew, иначе UPDATE.
    bool save(ProductKey& key);

    /// Удаляет ключ по id.
    bool remove(int id);

    /// Атомарно резервирует один свободный ключ под заказ.
    /// Возвращает зарезервированный ключ (status=Reserved) или ProductKey() при отсутствии.
    /// Используется при создании заказа.
    ProductKey reserveOneAvailable(int productId);

    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;
};