#include "DigitalShopAWS.h"
#include "database/DatabaseManager.h"

#include <QtWidgets/QApplication>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // Инициализация базы данных.
    // При первом запуске будет создан файл digitalshop.db и таблицы.
    if (!DatabaseManager::instance().initialize()) {
        QMessageBox::critical(nullptr,
            "Ошибка запуска",
            "Не удалось инициализировать базу данных.\n"
            "Подробности в логе приложения.");
        return 1;
    }

    DigitalShopAWS w;
    w.show();
    return a.exec();
}