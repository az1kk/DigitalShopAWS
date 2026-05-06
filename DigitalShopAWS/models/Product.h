#pragma once

#include <QString>

/**
 * @brief Модель товара.
 *
 * Простой класс данных без логики БД.
 * Содержит только поля и геттеры/сеттеры.
 *
 * Связан с таблицей products в БД.
 * Загрузкой/сохранением занимается ProductRepository.
 */
class Product
{
public:
    Product() = default;
    Product(int id, int categoryId, const QString& name,
        const QString& description, double price);

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    int categoryId() const { return m_categoryId; }
    void setCategoryId(int categoryId) { m_categoryId = categoryId; }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    QString description() const { return m_description; }
    void setDescription(const QString& description) { m_description = description; }

    double price() const { return m_price; }
    void setPrice(double price) { m_price = price; }

    /// true — если объект ещё не сохранён в БД (id == 0).
    bool isNew() const { return m_id == 0; }

private:
    int     m_id = 0;       // 0 = новый, ещё не в БД
    int     m_categoryId = 0;       // 0 = без категории
    QString m_name;
    QString m_description;
    double  m_price = 0.0;
};