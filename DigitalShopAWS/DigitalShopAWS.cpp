#include "ui/CustomersTab.h"
#include "ui/ProductsTab.h"
#include "ui/KeysTab.h"
#include "ui/OrdersTab.h"
#include "ui/AnalyticsTab.h"
#include "ui/StatisticsTab.h"
#include "DigitalShopAWS.h"

#include <QTabWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include <QMessageBox>

DigitalShopAWS::DigitalShopAWS(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();
}

void DigitalShopAWS::setupUi()
{
    setWindowTitle("АРМ — Управление торговлей цифровыми товарами");
    resize(1100, 700);

    setupMenuBar();
    setupTabs();
    setupStatusBar();
}

void DigitalShopAWS::setupMenuBar()
{
    QMenuBar* menu = menuBar();

    // Меню "Файл"
    QMenu* fileMenu = menu->addMenu("&Файл");

    QAction* exitAction = fileMenu->addAction("Вы&ход");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    // Меню "Справка"
    QMenu* helpMenu = menu->addMenu("&Справка");

    QAction* aboutAction = helpMenu->addAction("О &программе");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this,
            "О программе",
            "<b>АРМ менеджера цифровых товаров</b><br><br>"
            "Учебный проект по дисциплине \"Информатика\".<br>"
            "Версия 1.0");
        });
}

void DigitalShopAWS::setupTabs()
{
    m_tabs = new QTabWidget(this);
    setCentralWidget(m_tabs);

    auto makePlaceholder = [](const QString& text) {
        QWidget* w = new QWidget;
        QVBoxLayout* layout = new QVBoxLayout(w);
        QLabel* label = new QLabel(text);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: gray; font-size: 16px;");
        layout->addWidget(label);
        return w;
        };

    // Реальная вкладка "Товары"
    m_tabs->addTab(new ProductsTab(this), "Товары");
    m_tabs->addTab(new CustomersTab(this), "Клиенты");
    m_tabs->addTab(new KeysTab(this), "Ключи");
    m_tabs->addTab(new OrdersTab(this), QString::fromUtf8("Заказы"));
    m_tabs->addTab(new StatisticsTab(this), QString::fromUtf8("Статистика"));
    m_tabs->addTab(new AnalyticsTab(this), QString::fromUtf8("Аналитика"));
}

void DigitalShopAWS::setupStatusBar()
{
    statusBar()->showMessage("Готово");
}