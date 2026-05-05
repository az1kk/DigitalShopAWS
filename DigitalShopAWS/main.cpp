#include "DigitalShopAWS.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    DigitalShopAWS window;
    window.show();
    return app.exec();
}
