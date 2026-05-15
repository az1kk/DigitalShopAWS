#include "ui/CustomersTab.h"
#include "ui/ProductsTab.h"
#include "ui/KeysTab.h"
#include "ui/OrdersTab.h"
#include "ui/AnalyticsTab.h"
#include "ui/StatisticsTab.h"
#include "DigitalShopAWS.h"

#include <QTabWidget>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QIcon>

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

    // === Меню "Файл" ===
    QMenu* fileMenu = menu->addMenu(QString::fromUtf8("&Файл"));

    QAction* exitAction = fileMenu->addAction(QString::fromUtf8("Вы&ход"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    // === Меню "Справка" ===
    QMenu* helpMenu = menu->addMenu(QString::fromUtf8("&Справка"));

    QAction* aboutAction = helpMenu->addAction(QString::fromUtf8("О &программе"));
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox about(this);
        about.setWindowTitle(QString::fromUtf8("О программе"));
        about.setIconPixmap(QIcon(":/icons/app.ico").pixmap(64, 64));
        about.setTextFormat(Qt::RichText);
        about.setText(QString::fromUtf8(
            "<h3>DigitalShopAWS</h3>"
            "<p><b>АРМ менеджера магазина цифровых товаров</b></p>"
            "<p>Версия 1.0</p>"
            "<hr>"
            "<p><b>Авторы:</b><br>"
            "Азьмухаметов Руслан<br>"
            "Макаров Максим</p>"
            "<p><b>Дисциплина:</b> Информатика<br>"
            "<b>Тип работы:</b> творческая (групповая)<br>"
            "<b>Год:</b> 2026</p>"
            "<hr>"
            "<p><b>Технологии:</b> C++17, Qt 6, SQLite, OpenGL 3.3</p>"
        ));
        about.setStandardButtons(QMessageBox::Ok);
        about.exec();
        });

    QAction* aboutQtAction = helpMenu->addAction(QString::fromUtf8("О &Qt"));
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
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