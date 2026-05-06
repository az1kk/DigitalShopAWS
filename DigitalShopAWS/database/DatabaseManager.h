#pragma once

#include <QString>
#include <QSqlDatabase>

/**
 * @brief Менеджер подключения к базе данных SQLite.
 *
 * Реализует паттерн "Singleton" — один экземпляр на всё приложение.
 * Отвечает за:
 *   - открытие соединения с файлом БД при старте;
 *   - создание таблиц из schema.sql при первом запуске;
 *   - предоставление готового подключения другим классам.
 *
 * Использование:
 *   DatabaseManager::instance().initialize();
 *   QSqlDatabase db = DatabaseManager::instance().database();
 */
class DatabaseManager
{
public:
    /// Возвращает единственный экземпляр менеджера.
    static DatabaseManager& instance();

    /// Открывает БД и создаёт таблицы при необходимости.
    /// Возвращает true при успехе, false при ошибке.
    bool initialize(const QString& dbFilePath = "digitalshop.db");

    /// Возвращает текущее подключение к БД.
    QSqlDatabase database() const;

    /// Возвращает true, если БД успешно открыта.
    bool isOpen() const;

    /// Закрывает соединение. Вызывается при завершении работы.
    void close();

private:
    DatabaseManager() = default;
    ~DatabaseManager() = default;

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    /// Выполняет SQL-скрипт из ресурса :/sql/schema.sql.
    bool runSchema();

    QSqlDatabase m_db;
    bool m_initialized = false;
};