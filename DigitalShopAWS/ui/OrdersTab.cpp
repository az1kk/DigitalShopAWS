#include "OrdersTab.h"
#include "CreateOrderDialog.h"

#include "../repositories/OrderRepository.h"
#include "../repositories/CustomerRepository.h"
#include "../repositories/ProductRepository.h"
#include "../repositories/KeyRepository.h"
#include "../models/Order.h"
#include "../models/ProductKey.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>
#include <QHash>

namespace {

    QString statusText(Order::Status s)
    {
        switch (s) {
        case Order::Status::Pending:   return QString::fromUtf8("Ожидает оплаты");
        case Order::Status::Paid:      return QString::fromUtf8("Оплачен");
        case Order::Status::Cancelled: return QString::fromUtf8("Отменён");
        }
        return "?";
    }

    QColor statusColor(Order::Status s)
    {
        switch (s) {
        case Order::Status::Pending:   return QColor(255, 245, 200);  // жёлтый
        case Order::Status::Paid:      return QColor(220, 255, 220);  // зелёный
        case Order::Status::Cancelled: return QColor(230, 230, 230);  // серый
        }
        return Qt::white;
    }

} // namespace

OrdersTab::OrdersTab(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    refresh();
}

void OrdersTab::setupUi()
{
    m_statusFilter = new QComboBox(this);
    m_statusFilter->addItem(QString::fromUtf8("Все статусы"), -1);
    m_statusFilter->addItem(QString::fromUtf8("Ожидают оплаты"),
        static_cast<int>(Order::Status::Pending));
    m_statusFilter->addItem(QString::fromUtf8("Оплаченные"),
        static_cast<int>(Order::Status::Paid));
    m_statusFilter->addItem(QString::fromUtf8("Отменённые"),
        static_cast<int>(Order::Status::Cancelled));

    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({
        "ID",
        QString::fromUtf8("Дата"),
        QString::fromUtf8("Клиент"),
        QString::fromUtf8("Товар"),
        QString::fromUtf8("Цена"),
        QString::fromUtf8("Статус"),
        QString::fromUtf8("Ключ")
        });
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setColumnWidth(0, 50);
    m_table->setColumnWidth(1, 140);
    m_table->setColumnWidth(2, 200);
    m_table->setColumnWidth(3, 230);
    m_table->setColumnWidth(4, 100);
    m_table->setColumnWidth(5, 150);

    m_createBtn = new QPushButton(QString::fromUtf8("Создать заказ"), this);
    m_confirmBtn = new QPushButton(QString::fromUtf8("Подтвердить оплату"), this);
    m_cancelBtn = new QPushButton(QString::fromUtf8("Отменить"), this);
    m_refreshBtn = new QPushButton(QString::fromUtf8("Обновить"), this);

    QHBoxLayout* filterRow = new QHBoxLayout;
    filterRow->addWidget(new QLabel(QString::fromUtf8("Фильтр по статусу:")));
    filterRow->addWidget(m_statusFilter, 1);

    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->addWidget(m_createBtn);
    buttons->addWidget(m_confirmBtn);
    buttons->addWidget(m_cancelBtn);
    buttons->addStretch();
    buttons->addWidget(m_refreshBtn);

    QVBoxLayout* main = new QVBoxLayout(this);
    main->addWidget(new QLabel(QString::fromUtf8("<b>Заказы</b>")));
    main->addLayout(filterRow);
    main->addWidget(m_table);
    main->addLayout(buttons);

    connect(m_createBtn, &QPushButton::clicked, this, &OrdersTab::onCreateClicked);
    connect(m_confirmBtn, &QPushButton::clicked, this, &OrdersTab::onConfirmClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &OrdersTab::onCancelClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &OrdersTab::refresh);

    connect(m_statusFilter, &QComboBox::currentIndexChanged,
        this, &OrdersTab::onFilterChanged);
}

void OrdersTab::onFilterChanged()
{
    refresh();
}

void OrdersTab::refresh()
{
    OrderRepository      orderRepo;
    CustomerRepository   customerRepo;
    ProductRepository    productRepo;
    KeyRepository        keyRepo;

    // Кэш имён клиентов и товаров — чтобы не дёргать БД на каждую строку.
    QHash<int, QString> customerNames;
    for (const Customer& c : customerRepo.getAll()) {
        customerNames.insert(c.id(), c.name());
    }
    QHash<int, QString> productNames;
    for (const Product& p : productRepo.getAll()) {
        productNames.insert(p.id(), p.name());
    }

    QList<Order> orders = orderRepo.getAll();

    // Фильтр по статусу.
    const int filterStatus = m_statusFilter->currentData().toInt();
    if (filterStatus >= 0) {
        QList<Order> filtered;
        const auto target = static_cast<Order::Status>(filterStatus);
        for (const Order& o : orders) {
            if (o.status() == target) {
                filtered.append(o);
            }
        }
        orders = filtered;
    }

    m_table->setRowCount(orders.size());

    for (int row = 0; row < orders.size(); ++row) {
        const Order& o = orders[row];

        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(o.id()));
        idItem->setData(Qt::UserRole, o.id());
        m_table->setItem(row, 0, idItem);

        m_table->setItem(row, 1, new QTableWidgetItem(
            o.createdAt().toString("dd.MM.yyyy HH:mm")));
        m_table->setItem(row, 2, new QTableWidgetItem(
            customerNames.value(o.customerId(), "?")));
        m_table->setItem(row, 3, new QTableWidgetItem(
            productNames.value(o.productId(), "?")));
        m_table->setItem(row, 4, new QTableWidgetItem(
            QString::number(o.price(), 'f', 2) + " " + QString::fromUtf8("₽")));

        QTableWidgetItem* statusItem = new QTableWidgetItem(statusText(o.status()));
        statusItem->setBackground(statusColor(o.status()));
        statusItem->setForeground(QColor(40, 40, 40));
        m_table->setItem(row, 5, statusItem);

        // Значение ключа показываем только для оплаченных заказов —
        // у "ожидающих оплаты" ключ ещё не выдан клиенту, и оператор
        // не должен видеть его в общей таблице.
        QString keyText;
        if (o.status() == Order::Status::Paid) {
            ProductKey k = keyRepo.getById(o.keyId());
            keyText = k.keyValue();
        }
        else if (o.status() == Order::Status::Pending) {
            keyText = QString::fromUtf8("(резерв)");
        }
        else {
            keyText = "—";
        }
        m_table->setItem(row, 6, new QTableWidgetItem(keyText));
    }
}

int OrdersTab::selectedOrderId() const
{
    const auto rows = m_table->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        return 0;
    }
    QTableWidgetItem* item = m_table->item(rows.first().row(), 0);
    return item ? item->data(Qt::UserRole).toInt() : 0;
}

void OrdersTab::onCreateClicked()
{
    CreateOrderDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    OrderRepository repo;
    Order o = repo.createOrder(dlg.selectedCustomerId(),
        dlg.selectedProductId());
    if (o.id() == 0) {
        QMessageBox::warning(this, QString::fromUtf8("Ошибка"),
            QString::fromUtf8("Не удалось создать заказ:\n") + repo.lastError());
        return;
    }

    QMessageBox::information(this, QString::fromUtf8("Заказ создан"),
        QString::fromUtf8("Создан заказ №%1.\nОжидает оплаты.").arg(o.id()));

    refresh();
}

void OrdersTab::onConfirmClicked()
{
    const int id = selectedOrderId();
    if (id == 0) {
        QMessageBox::information(this, QString::fromUtf8("Внимание"),
            QString::fromUtf8("Выберите заказ"));
        return;
    }

    OrderRepository repo;
    Order o = repo.getById(id);

    if (o.status() != Order::Status::Pending) {
        QMessageBox::warning(this, QString::fromUtf8("Нельзя"),
            QString::fromUtf8("Подтвердить оплату можно только для заказов в статусе "
                "\"ожидает оплаты\"."));
        return;
    }

    const auto answer = QMessageBox::question(this,
        QString::fromUtf8("Подтверждение"),
        QString::fromUtf8("Подтвердить оплату заказа №%1?\n"
            "После этого ключ будет помечен как проданный.").arg(id),
        QMessageBox::Yes | QMessageBox::No);

    if (answer != QMessageBox::Yes) {
        return;
    }

    if (!repo.confirmPayment(id)) {
        QMessageBox::warning(this, QString::fromUtf8("Ошибка"),
            QString::fromUtf8("Не удалось подтвердить оплату:\n") + repo.lastError());
        return;
    }

    // После подтверждения показываем клиенту его ключ.
    Order updated = repo.getById(id);
    KeyRepository keyRepo;
    ProductKey key = keyRepo.getById(updated.keyId());

    QMessageBox::information(this, QString::fromUtf8("Заказ выполнен"),
        QString::fromUtf8("Заказ №%1 оплачен.\n\nКлюч для выдачи клиенту:\n%2")
        .arg(id).arg(key.keyValue()));

    refresh();
}

void OrdersTab::onCancelClicked()
{
    const int id = selectedOrderId();
    if (id == 0) {
        QMessageBox::information(this, QString::fromUtf8("Внимание"),
            QString::fromUtf8("Выберите заказ"));
        return;
    }

    OrderRepository repo;
    Order o = repo.getById(id);

    if (o.status() == Order::Status::Cancelled) {
        QMessageBox::information(this, QString::fromUtf8("Внимание"),
            QString::fromUtf8("Заказ уже отменён"));
        return;
    }

    QString message;
    if (o.status() == Order::Status::Paid) {
        message = QString::fromUtf8(
            "Отменить оплаченный заказ №%1?\n"
            "Ключ будет возвращён в свободные и сможет быть продан заново.").arg(id);
    }
    else {
        message = QString::fromUtf8(
            "Отменить заказ №%1?\n"
            "Зарезервированный ключ вернётся в свободные.").arg(id);
    }

    const auto answer = QMessageBox::question(this,
        QString::fromUtf8("Подтверждение"), message,
        QMessageBox::Yes | QMessageBox::No);

    if (answer != QMessageBox::Yes) {
        return;
    }

    if (!repo.cancelOrder(id)) {
        QMessageBox::warning(this, QString::fromUtf8("Ошибка"),
            QString::fromUtf8("Не удалось отменить заказ:\n") + repo.lastError());
        return;
    }

    refresh();
}