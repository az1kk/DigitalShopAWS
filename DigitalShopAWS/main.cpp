#include "DigitalShopAWS.h"
#include "database/DatabaseManager.h"
#include "ui/LoginDialog.h"

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

    // Окно входа. Если пользователь нажал Отмена — выходим из приложения.
    LoginDialog login;
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    DigitalShopAWS w;
    w.show();
    return a.exec();
}