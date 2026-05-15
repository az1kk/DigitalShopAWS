#pragma once

#include "../models/Customer.h"
#include <QList>
#include <QString>

/**
 * @brief Репозиторий клиентов.
 *
 * Отвечает за загрузку и сохранение объектов Customer в таблицу customers.
 * Использует подключение из DatabaseManager::instance().
 */
class CustomerRepository
{
public:
    /// Возвращает всех клиентов, отсортированных по имени.
    QList<Customer> getAll();

    /// Находит клиента по id. Если не найден, возвращает Customer() с id=0.
    Customer getById(int id);

    /// Сохраняет клиента. Если customer.isNew() — INSERT, иначе UPDATE.
    /// При успешном INSERT обновляет customer.setId(...) присвоенным id.
    bool save(Customer& customer);

    /// Удаляет клиента по id.
    bool remove(int id);

    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;
};
