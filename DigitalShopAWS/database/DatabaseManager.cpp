#include "DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QFileInfo>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager s_instance;
    return s_instance;
}

bool DatabaseManager::initialize(const QString& dbFilePath)
{
    if (m_initialized) {
        return true;
    }

    const bool dbExisted = QFileInfo::exists(dbFilePath);

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbFilePath);

    if (!m_db.open()) {
        qCritical() << "Не удалось открыть БД:" << m_db.lastError().text();
        return false;
    }

    if (!dbExisted) {
        qInfo() << "Файл БД не найден, создаю таблицы из schema.sql";
        if (!runSchema()) {
            qCritical() << "Ошибка при создании схемы БД";
            m_db.close();
            return false;
        }
    }

    m_initialized = true;
    qInfo() << "БД успешно инициализирована:" << dbFilePath;
    return true;
}

QSqlDatabase DatabaseManager::database() const
{
    return m_db;
}

bool DatabaseManager::isOpen() const
{
    return m_initialized && m_db.isOpen();
}

void DatabaseManager::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    m_initialized = false;
}

bool DatabaseManager::runSchema()
{
    QFile schemaFile(":/sql/schema.sql");
    if (!schemaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Не удалось открыть ресурс :/sql/schema.sql";
        return false;
    }

    QTextStream in(&schemaFile);
    const QString fullScript = in.readAll();

    const QStringList statements = fullScript.split(';', Qt::SkipEmptyParts);

    QSqlQuery query(m_db);
    for (const QString& stmt : statements) {
        const QString trimmed = stmt.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }
        if (!query.exec(trimmed)) {
            qCritical() << "Ошибка выполнения SQL:" << query.lastError().text()
                << "\nЗапрос:" << trimmed;
            return false;
        }
    }

    return true;
}