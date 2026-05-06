#include "PasswordHasher.h"

#include <QCryptographicHash>

QString PasswordHasher::hash(const QString& password)
{
    const QByteArray bytes = password.toUtf8();
    const QByteArray digest = QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);
    return QString::fromLatin1(digest.toHex());
}

bool PasswordHasher::verify(const QString& password, const QString& expectedHash)
{
    return hash(password).compare(expectedHash, Qt::CaseInsensitive) == 0;
}