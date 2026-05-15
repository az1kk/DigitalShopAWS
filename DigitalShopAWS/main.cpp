#include "DigitalShopAWS.h"
#include "database/DatabaseManager.h"
#include "ui/LoginDialog.h"
#include "models/Customer.h"
#include "models/ProductKey.h"
#include "repositories/CustomerRepository.h"
#include "repositories/KeyRepository.h"
#include "repositories/ProductRepository.h"

#include <QtWidgets/QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QIcon>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/resources/app.ico"));

    if (!DatabaseManager::instance().initialize()) {
        QMessageBox::critical(nullptr, "Ошибка запуска",
            "Не удалось инициализировать базу данных.");
        return 1;
    }

    // ====== ТЕСТ работы напарника ======
    qInfo() << "=== Тест CustomerRepository ===";
    {
        CustomerRepository repo;

        Customer c(0, "Иван Иванов", "ivan@example.com", "+79991234567");
        if (repo.save(c)) {
            qInfo() << "Создан клиент id=" << c.id() << "имя=" << c.name();
        }
        else {
            qWarning() << "Ошибка сохранения клиента:" << repo.lastError();
        }

        const auto all = repo.getAll();
        qInfo() << "Всего клиентов в БД:" << all.size();
        for (const Customer& x : all) {
            qInfo() << "  id=" << x.id() << " name=" << x.name()
                << " email=" << x.email() << " phone=" << x.phone();
        }
    }

    qInfo() << "";
    qInfo() << "=== Тест KeyRepository ===";
    {
        ProductRepository prodRepo;
        const auto products = prodRepo.getAll();
        if (products.isEmpty()) {
            qWarning() << "В БД нет товаров — пропускаю тест ключей";
        }
        else {
            const int productId = products.first().id();
            qInfo() << "Использую товар id=" << productId
                << " name=" << products.first().name();

            KeyRepository keyRepo;

            // Создаём два тестовых ключа.
            ProductKey k1(0, productId, "TEST-KEY-AAAA-1111",
                ProductKey::Status::Available);
            ProductKey k2(0, productId, "TEST-KEY-BBBB-2222",
                ProductKey::Status::Available);

            if (keyRepo.save(k1))
                qInfo() << "Создан ключ id=" << k1.id();
            if (keyRepo.save(k2))
                qInfo() << "Создан ключ id=" << k2.id();

            qInfo() << "Доступных ключей до резерва:"
                << keyRepo.countAvailable(productId);

            // Тестируем резервирование.
            ProductKey reserved = keyRepo.reserveOneAvailable(productId);
            if (reserved.id() > 0) {
                qInfo() << "Зарезервирован ключ id=" << reserved.id()
                    << " value=" << reserved.keyValue();
            }
            else {
                qWarning() << "Резервирование не удалось:" << keyRepo.lastError();
            }

            qInfo() << "Доступных ключей после резерва:"
                << keyRepo.countAvailable(productId);
        }
    }
    // ====== конец теста ======

    LoginDialog login;
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    DigitalShopAWS w;
    w.show();
    return a.exec();
}