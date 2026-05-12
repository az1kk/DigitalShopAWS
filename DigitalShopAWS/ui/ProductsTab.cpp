#include "ProductsTab.h"
#include "ProductEditDialog.h"

#include "../repositories/ProductRepository.h"
#include "../models/Product.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>

ProductsTab::ProductsTab(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    refresh();
}

void ProductsTab::setupUi()
{
    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({ "ID", "Название", "Цена", "Описание" });
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);   // редактируем только через диалог
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);  // выделение целыми строками
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setColumnWidth(0, 50);
    m_table->setColumnWidth(1, 280);
    m_table->setColumnWidth(2, 120);

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
    main->addWidget(new QLabel("<b>Каталог товаров</b>"));
    main->addWidget(m_table);
    main->addLayout(buttons);

    connect(m_addBtn, &QPushButton::clicked, this, &ProductsTab::onAddClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &ProductsTab::onEditClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &ProductsTab::onDeleteClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &ProductsTab::refresh);

    // Двойной клик по строке = редактировать
    connect(m_table, &QTableWidget::doubleClicked,
        this, &ProductsTab::onEditClicked);
}

void ProductsTab::refresh()
{
    ProductRepository repo;
    const QList<Product> products = repo.getAll();

    m_table->setRowCount(products.size());

    for (int row = 0; row < products.size(); ++row) {
        const Product& p = products[row];

        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(p.id()));
        idItem->setData(Qt::UserRole, p.id());          // храним id в UserRole
        m_table->setItem(row, 0, idItem);

        m_table->setItem(row, 1, new QTableWidgetItem(p.name()));
        m_table->setItem(row, 2, new QTableWidgetItem(
            QString::number(p.price(), 'f', 2) + " ₽"));
        m_table->setItem(row, 3, new QTableWidgetItem(p.description()));
    }
}

int ProductsTab::selectedProductId() const
{
    const auto rows = m_table->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        return 0;
    }
    // Достаём id из UserRole первой колонки.
    QTableWidgetItem* item = m_table->item(rows.first().row(), 0);
    return item ? item->data(Qt::UserRole).toInt() : 0;
}

void ProductsTab::onAddClicked()
{
    ProductEditDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    Product p = dlg.product();
    ProductRepository repo;
    if (!repo.save(p)) {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось сохранить товар:\n" + repo.lastError());
        return;
    }

    refresh();
}

void ProductsTab::onEditClicked()
{
    const int id = selectedProductId();
    if (id == 0) {
        QMessageBox::information(this, "Внимание",
            "Выберите товар для редактирования");
        return;
    }

    ProductRepository repo;
    Product p = repo.getById(id);
    if (p.isNew()) {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось загрузить товар");
        return;
    }

    ProductEditDialog dlg(this);
    dlg.setProduct(p);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    Product updated = dlg.product();
    if (!repo.save(updated)) {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось сохранить товар:\n" + repo.lastError());
        return;
    }

    refresh();
}

void ProductsTab::onDeleteClicked()
{
    const int id = selectedProductId();
    if (id == 0) {
        QMessageBox::information(this, "Внимание",
            "Выберите товар для удаления");
        return;
    }

    ProductRepository repo;
    Product p = repo.getById(id);

    const auto answer = QMessageBox::question(this,
        "Подтверждение",
        QString("Удалить товар \"%1\"?").arg(p.name()),
        QMessageBox::Yes | QMessageBox::No);

    if (answer != QMessageBox::Yes) {
        return;
    }

    if (!repo.remove(id)) {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось удалить товар:\n" + repo.lastError());
        return;
    }

    refresh();
}