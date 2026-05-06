#include "DigitalShopAWS.h"
#include "database/DatabaseManager.h"

#include <QtWidgets/QApplication>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    if (!DatabaseManager::instance().initialize()) {
        QMessageBox::critical(nullptr,
            "Ошибка запуска",
            "Не удалось инициализировать базу данных.");
        return 1;
    }

    DigitalShopAWS w;
    w.show();
    return a.exec();
}