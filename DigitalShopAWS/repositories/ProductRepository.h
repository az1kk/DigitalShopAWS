#pragma once

#include "../models/Product.h"
#include <QList>
#include <QString>

/**
 * @brief Репозиторий товаров.
 *
 * Отвечает за загрузку и сохранение объектов Product в таблицу products.
 * Не содержит UI-логики и не знает о виджетах.
 *
 * Использует подключение из DatabaseManager::instance().
 */
class ProductRepository
{
public:
    /// Возвращает все товары, отсортированные по имени.
    QList<Product> getAll();

    /// Находит товар по id. Если не найден, возвращает Product() c id=0.
    Product getById(int id);

    /// Сохраняет товар. Если product.isNew() — INSERT, иначе UPDATE.
    /// При успешном INSERT обновляет product.setId(...) присвоенным id.
    /// Возвращает true при успехе.
    bool save(Product& product);

    /// Удаляет товар по id. Возвращает true при успехе.
    bool remove(int id);

    /// Возвращает строку с последней ошибкой (для отображения пользователю).
    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;
};