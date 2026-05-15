#include "StatisticsTab.h"

#include "../repositories/ProductRepository.h"
#include "../repositories/CustomerRepository.h"
#include "../repositories/KeyRepository.h"
#include "../repositories/OrderRepository.h"
#include "../models/ProductKey.h"
#include "../models/Order.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

StatisticsTab::StatisticsTab(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    refresh();
}

void StatisticsTab::setupUi()
{
    // Вспомогательная функция: создаёт жирную надпись для значения.
    auto makeValueLabel = []() {
        QLabel* label = new QLabel("0");
        QFont f = label->font();
        f.setBold(true);
        f.setPointSize(f.pointSize() + 1);
        label->setFont(f);
        return label;
        };

    m_productsLabel = makeValueLabel();
    m_customersLabel = makeValueLabel();

    m_keysTotalLabel = makeValueLabel();
    m_keysAvailableLabel = makeValueLabel();
    m_keysReservedLabel = makeValueLabel();
    m_keysSoldLabel = makeValueLabel();

    m_ordersTotalLabel = makeValueLabel();
    m_ordersPendingLabel = makeValueLabel();
    m_ordersPaidLabel = makeValueLabel();
    m_ordersCancelledLabel = makeValueLabel();

    m_revenueLabel = makeValueLabel();

    // --- Группа "Каталог" ---
    QGroupBox* catalogBox = new QGroupBox(QString::fromUtf8("Каталог"));
    QFormLayout* catalogForm = new QFormLayout(catalogBox);
    catalogForm->addRow(QString::fromUtf8("Всего товаров:"), m_productsLabel);
    catalogForm->addRow(QString::fromUtf8("Всего клиентов:"), m_customersLabel);

    // --- Группа "Склад ключей" ---
    QGroupBox* keysBox = new QGroupBox(QString::fromUtf8("Склад ключей"));
    QFormLayout* keysForm = new QFormLayout(keysBox);
    keysForm->addRow(QString::fromUtf8("Всего ключей:"), m_keysTotalLabel);
    keysForm->addRow(QString::fromUtf8("Доступно:"), m_keysAvailableLabel);
    keysForm->addRow(QString::fromUtf8("Зарезервировано:"), m_keysReservedLabel);
    keysForm->addRow(QString::fromUtf8("Продано:"), m_keysSoldLabel);

    // --- Группа "Заказы" ---
    QGroupBox* ordersBox = new QGroupBox(QString::fromUtf8("Заказы"));
    QFormLayout* ordersForm = new QFormLayout(ordersBox);
    ordersForm->addRow(QString::fromUtf8("Всего заказов:"), m_ordersTotalLabel);
    ordersForm->addRow(QString::fromUtf8("Ожидают оплаты:"), m_ordersPendingLabel);
    ordersForm->addRow(QString::fromUtf8("Оплачено:"), m_ordersPaidLabel);
    ordersForm->addRow(QString::fromUtf8("Отменено:"), m_ordersCancelledLabel);

    // --- Группа "Финансы" ---
    QGroupBox* financeBox = new QGroupBox(QString::fromUtf8("Финансы"));
    QFormLayout* financeForm = new QFormLayout(financeBox);
    financeForm->addRow(QString::fromUtf8("Общая выручка:"), m_revenueLabel);

    // Раскладываем группы в две колонки.
    QHBoxLayout* columns = new QHBoxLayout;

    QVBoxLayout* leftColumn = new QVBoxLayout;
    leftColumn->addWidget(catalogBox);
    leftColumn->addWidget(keysBox);
    leftColumn->addStretch();

    QVBoxLayout* rightColumn = new QVBoxLayout;
    rightColumn->addWidget(ordersBox);
    rightColumn->addWidget(financeBox);
    rightColumn->addStretch();

    columns->addLayout(leftColumn);
    columns->addLayout(rightColumn);

    m_refreshBtn = new QPushButton(QString::fromUtf8("Обновить"), this);

    QHBoxLayout* buttonRow = new QHBoxLayout;
    buttonRow->addStretch();
    buttonRow->addWidget(m_refreshBtn);

    QVBoxLayout* main = new QVBoxLayout(this);
    QLabel* title = new QLabel(QString::fromUtf8("<b>Сводная статистика</b>"));
    main->addWidget(title);
    main->addLayout(columns);
    main->addStretch();
    main->addLayout(buttonRow);

    connect(m_refreshBtn, &QPushButton::clicked, this, &StatisticsTab::refresh);
}

void StatisticsTab::refresh()
{
    // --- Каталог ---
    ProductRepository productRepo;
    CustomerRepository customerRepo;


    const auto products = productRepo.getAll();
    const auto customers = customerRepo.getAll();

    m_productsLabel->setText(QString::number(products.size()));
    m_customersLabel->setText(QString::number(customers.size()));

    // --- Ключи ---
    KeyRepository keyRepo;
    const auto allKeys = keyRepo.getAll();

    int keysAvailable = 0;
    int keysReserved = 0;
    int keysSold = 0;
    for (const ProductKey& k : allKeys) {
        switch (k.status()) {
        case ProductKey::Status::Available: ++keysAvailable; break;
        case ProductKey::Status::Reserved:  ++keysReserved;  break;
        case ProductKey::Status::Sold:      ++keysSold;      break;
        }
    }

    m_keysTotalLabel->setText(QString::number(allKeys.size()));
    m_keysAvailableLabel->setText(QString::number(keysAvailable));
    m_keysReservedLabel->setText(QString::number(keysReserved));
    m_keysSoldLabel->setText(QString::number(keysSold));

    // --- Заказы ---
    OrderRepository orderRepo;
    const auto allOrders = orderRepo.getAll();

    int ordersPending = 0;
    int ordersPaid = 0;
    int ordersCancelled = 0;
    double revenue = 0.0;

    for (const Order& o : allOrders) {
        switch (o.status()) {
        case Order::Status::Pending:   ++ordersPending;   break;
        case Order::Status::Paid:
            ++ordersPaid;
            revenue += o.price();   // выручка — только по оплаченным
            break;
        case Order::Status::Cancelled: ++ordersCancelled; break;
        }
    }

    m_ordersTotalLabel->setText(QString::number(allOrders.size()));
    m_ordersPendingLabel->setText(QString::number(ordersPending));
    m_ordersPaidLabel->setText(QString::number(ordersPaid));
    m_ordersCancelledLabel->setText(QString::number(ordersCancelled));

    // --- Финансы ---
    m_revenueLabel->setText(
        QString::number(revenue, 'f', 2) + " " + QString::fromUtf8("₽"));
}