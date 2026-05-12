#pragma once

#include <QtWidgets/QMainWindow>

class QTabWidget;

/**
 * @brief Главное окно АРМ.
 *
 * Содержит вкладки для управления товарами, ключами, клиентами,
 * заказами и аналитикой. Каждая вкладка — отдельный виджет.
 */
class DigitalShopAWS : public QMainWindow
{
    Q_OBJECT

public:
    explicit DigitalShopAWS(QWidget* parent = nullptr);
    ~DigitalShopAWS() = default;

private:
    void setupUi();
    void setupMenuBar();
    void setupStatusBar();
    void setupTabs();

    QTabWidget* m_tabs = nullptr;
};