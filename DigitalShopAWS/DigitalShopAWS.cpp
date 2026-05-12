#include <QMessageBox>
#include "DigitalShopAWS.h"

#include <QTabWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>

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

    // Создаём вкладки-заглушки. Каждая будет заменена на реальный
    // функциональный виджет на следующих шагах.

    auto makePlaceholder = [](const QString& text) {
        QWidget* w = new QWidget;
        QVBoxLayout* layout = new QVBoxLayout(w);
        QLabel* label = new QLabel(text);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: gray; font-size: 16px;");
        layout->addWidget(label);
        return w;
        };

    m_tabs->addTab(makePlaceholder("Вкладка \"Товары\" — в разработке"), "Товары");
    m_tabs->addTab(makePlaceholder("Вкладка \"Ключи\" — в разработке"), "Ключи");
    m_tabs->addTab(makePlaceholder("Вкладка \"Клиенты\" — в разработке"), "Клиенты");
    m_tabs->addTab(makePlaceholder("Вкладка \"Заказы\" — в разработке"), "Заказы");
    m_tabs->addTab(makePlaceholder("Вкладка \"Аналитика\" — в разработке"), "Аналитика");
}

void DigitalShopAWS::setupStatusBar()
{
    statusBar()->showMessage("Готово");
}