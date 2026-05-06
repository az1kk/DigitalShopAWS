#pragma once

#include <QString>

/**
 * @brief Утилита хеширования паролей.
 *
 * Используем SHA-256 — простой и достаточный для учебного проекта вариант.
 * В реальных системах применяют адаптивные алгоритмы вроде bcrypt/argon2,
 * но для АРМ внутри организации SHA-256 покрывает основной риск:
 * утечку списка паролей через прямой доступ к БД.
 */
class PasswordHasher
{
public:
    /// Возвращает SHA-256 хеш в виде hex-строки нижним регистром.
    /// Например, hash("admin") даст
    /// "8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918"
    static QString hash(const QString& password);

    /// Проверяет, что хеш переданного пароля совпадает с эталонным.
    static bool verify(const QString& password, const QString& expectedHash);
};