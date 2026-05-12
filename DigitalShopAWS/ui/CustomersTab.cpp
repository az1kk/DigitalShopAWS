#include "CustomersTab.h"
#include "CustomerEditDialog.h"

#include "../repositories/CustomerRepository.h"
#include "../models/Customer.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>

CustomersTab::CustomersTab(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    refresh();
}

void CustomersTab::setupUi()
{
    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({ "ID", "Имя", "Email", "Телефон" });
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setColumnWidth(0, 50);
    m_table->setColumnWidth(1, 260);
    m_table->setColumnWidth(2, 260);

    m_addBtn = new QPushButton("Добавить", this);
    m_editBtn = new QPushButton("Редактировать", this);
    m_deleteBtn = new QPushButton("Удалить", this);
    m_refreshBtn = new QPushButton("Обновить", this);

    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->addWidget(m_addBtn);
    buttons->addWidget(m_editBtn);
    buttons->addWidget(m_deleteBtn);
    buttons->addStretch();
    buttons->addWidget(m_refreshBtn);

    QVBoxLayout* main = new QVBoxLayout(this);
    main->addWidget(new QLabel("<b>База клиентов</b>"));
    main->addWidget(m_table);
    main->addLayout(buttons);

    connect(m_addBtn, &QPushButton::clicked, this, &CustomersTab::onAddClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &CustomersTab::onEditClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &CustomersTab::onDeleteClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &CustomersTab::refresh);

    connect(m_table, &QTableWidget::doubleClicked,
        this, &CustomersTab::onEditClicked);
}

void CustomersTab::refresh()
{
    CustomerRepository repo;
    const QList<Customer> customers = repo.getAll();

    m_table->setRowCount(customers.size());

    for (int row = 0; row < customers.size(); ++row) {
        const Customer& c = customers[row];

        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(c.id()));
        idItem->setData(Qt::UserRole, c.id());
        m_table->setItem(row, 0, idItem);

        m_table->setItem(row, 1, new QTableWidgetItem(c.name()));
        m_table->setItem(row, 2, new QTableWidgetItem(c.email()));
        m_table->setItem(row, 3, new QTableWidgetItem(c.phone()));
    }
}

int CustomersTab::selectedCustomerId() const
{
    const auto rows = m_table->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        return 0;
    }
    QTableWidgetItem* item = m_table->item(rows.first().row(), 0);
    return item ? item->data(Qt::UserRole).toInt() : 0;
}

void CustomersTab::onAddClicked()
{
    CustomerEditDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    Customer c = dlg.customer();
    CustomerRepository repo;
    if (!repo.save(c)) {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось сохранить клиента:\n" + repo.lastError());
        return;
    }

    refresh();
}

void CustomersTab::onEditClicked()
{
    const int id = selectedCustomerId();
    if (id == 0) {
        QMessageBox::information(this, "Внимание",
            "Выберите клиента для редактирования");
        return;
    }

    CustomerRepository repo;
    Customer c = repo.getById(id);
    if (c.id() == 0) {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось загрузить клиента");
        return;
    }

    CustomerEditDialog dlg(this);
    dlg.setCustomer(c);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    Customer updated = dlg.customer();
    if (!repo.save(updated)) {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось сохранить клиента:\n" + repo.lastError());
        return;
    }

    refresh();
}

void CustomersTab::onDeleteClicked()
{
    const int id = selectedCustomerId();
    if (id == 0) {
        QMessageBox::information(this, "Внимание",
            "Выберите клиента для удаления");
        return;
    }

    CustomerRepository repo;
    Customer c = repo.getById(id);

    const auto answer = QMessageBox::question(this,
        "Подтверждение",
        QString("Удалить клиента \"%1\"?").arg(c.name()),
        QMessageBox::Yes | QMessageBox::No);

    if (answer != QMessageBox::Yes) {
        return;
    }

    if (!repo.remove(id)) {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось удалить клиента:\n" + repo.lastError());
        return;
    }

    refresh();
}