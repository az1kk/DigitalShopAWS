#include "KeysTab.h"
#include "KeyEditDialog.h"

#include "../repositories/KeyRepository.h"
#include "../repositories/ProductRepository.h"
#include "../models/ProductKey.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QLabel>
#include <QHash>

namespace {

    QString statusText(ProductKey::Status s)
    {
        switch (s) {
        case ProductKey::Status::Available: return "Доступен";
        case ProductKey::Status::Sold:      return "Продан";
        case ProductKey::Status::Reserved:  return "Зарезервирован";
        }
        return "?";
    }

    QColor statusTextColor(ProductKey::Status s)
    {
        // Тёмный текст для всех цветных фонов — читаем на любой системной теме.
        Q_UNUSED(s);
        return QColor(40, 40, 40);
    }

    QColor statusColor(ProductKey::Status s)
    {
        switch (s) {
        case ProductKey::Status::Available: return QColor(220, 255, 220);
        case ProductKey::Status::Sold:      return QColor(255, 220, 220);
        case ProductKey::Status::Reserved:  return QColor(255, 245, 200);
        }
        return Qt::white;
    }

} // namespace

KeysTab::KeysTab(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    reloadProductFilter();
    refresh();
}

void KeysTab::setupUi()
{
    m_productFilter = new QComboBox(this);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({ "ID", "Товар", "Ключ", "Статус", "Добавлен" });
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->setColumnWidth(0, 50);
    m_table->setColumnWidth(1, 260);
    m_table->setColumnWidth(2, 280);
    m_table->setColumnWidth(3, 140);

    m_addBtn = new QPushButton("Добавить", this);
    m_editBtn = new QPushButton("Редактировать", this);
    m_deleteBtn = new QPushButton("Удалить", this);
    m_refreshBtn = new QPushButton("Обновить", this);

    QHBoxLayout* filterRow = new QHBoxLayout;
    filterRow->addWidget(new QLabel("Фильтр по товару:"));
    filterRow->addWidget(m_productFilter, 1);

    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->addWidget(m_addBtn);
    buttons->addWidget(m_editBtn);
    buttons->addWidget(m_deleteBtn);
    buttons->addStretch();
    buttons->addWidget(m_refreshBtn);

    QVBoxLayout* main = new QVBoxLayout(this);
    main->addWidget(new QLabel("<b>Склад ключей</b>"));
    main->addLayout(filterRow);
    main->addWidget(m_table);
    main->addLayout(buttons);

    connect(m_addBtn, &QPushButton::clicked, this, &KeysTab::onAddClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &KeysTab::onEditClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &KeysTab::onDeleteClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &KeysTab::refresh);

    connect(m_productFilter, &QComboBox::currentIndexChanged,
        this, &KeysTab::onFilterChanged);

    connect(m_table, &QTableWidget::doubleClicked,
        this, &KeysTab::onEditClicked);
}

void KeysTab::reloadProductFilter()
{
    const int previousProductId = m_productFilter->currentData().toInt();

    m_productFilter->blockSignals(true);
    m_productFilter->clear();
    m_productFilter->addItem("Все товары", 0);

    ProductRepository repo;
    const auto products = repo.getAll();
    for (const Product& p : products) {
        m_productFilter->addItem(p.name(), p.id());
    }

    // Восстанавливаем выбор фильтра, если возможно.
    for (int i = 0; i < m_productFilter->count(); ++i) {
        if (m_productFilter->itemData(i).toInt() == previousProductId) {
            m_productFilter->setCurrentIndex(i);
            break;
        }
    }
    m_productFilter->blockSignals(false);
}

void KeysTab::onFilterChanged()
{
    refresh();
}

void KeysTab::refresh()
{
    // Загружаем все ключи и (для подписи) имена товаров.
    KeyRepository keyRepo;
    ProductRepository prodRepo;

    QList<ProductKey> keys = keyRepo.getAll();

    // Фильтрация по выбранному товару.
    const int filterId = m_productFilter->currentData().toInt();
    if (filterId > 0) {
        QList<ProductKey> filtered;
        for (const ProductKey& k : keys) {
            if (k.productId() == filterId) {
                filtered.append(k);
            }
        }
        keys = filtered;
    }

    // Кэш имён товаров: id -> name. Чтобы не дёргать БД по разу на ключ.
    QHash<int, QString> productNames;
    for (const Product& p : prodRepo.getAll()) {
        productNames.insert(p.id(), p.name());
    }

    m_table->setRowCount(keys.size());

    for (int row = 0; row < keys.size(); ++row) {
        const ProductKey& k = keys[row];

        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(k.id()));
        idItem->setData(Qt::UserRole, k.id());
        m_table->setItem(row, 0, idItem);

        m_table->setItem(row, 1, new QTableWidgetItem(
            productNames.value(k.productId(), "?")));
        m_table->setItem(row, 2, new QTableWidgetItem(k.keyValue()));

        QTableWidgetItem* statusItem = new QTableWidgetItem(statusText(k.status()));
        statusItem->setBackground(statusColor(k.status()));
        statusItem->setForeground(statusTextColor(k.status()));
        m_table->setItem(row, 3, statusItem);

        m_table->setItem(row, 4, new QTableWidgetItem(
            k.addedAt().toString("dd.MM.yyyy HH:mm")));
    }
}

int KeysTab::selectedKeyId() const
{
    const auto rows = m_table->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        return 0;
    }
    QTableWidgetItem* item = m_table->item(rows.first().row(), 0);
    return item ? item->data(Qt::UserRole).toInt() : 0;
}

void KeysTab::onAddClicked()
{
    // Каждый раз перед открытием обновим список товаров —
    // вдруг на вкладке "Товары" что-то добавили.
    reloadProductFilter();

    KeyEditDialog dlg(this);
    dlg.setAddMode();
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    const int productId = dlg.selectedProductId();
    const QStringList values = dlg.keyValues();

    KeyRepository repo;
    int saved = 0;
    int failed = 0;

    for (const QString& v : values) {
        ProductKey k(0, productId, v, ProductKey::Status::Available);
        if (repo.save(k)) {
            ++saved;
        }
        else {
            ++failed;
        }
    }

    if (failed > 0) {
        QMessageBox::warning(this, "Результат",
            QString("Сохранено: %1\nС ошибкой: %2\nПоследняя ошибка: %3")
            .arg(saved).arg(failed).arg(repo.lastError()));
    }
    else {
        QMessageBox::information(this, "Готово",
            QString("Добавлено ключей: %1").arg(saved));
    }

    refresh();
}

void KeysTab::onEditClicked()
{
    const int id = selectedKeyId();
    if (id == 0) {
        QMessageBox::information(this, "Внимание",
            "Выберите ключ для редактирования");
        return;
    }

    KeyRepository repo;
    ProductKey k = repo.getById(id);
    if (k.id() == 0) {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось загрузить ключ");
        return;
    }

    KeyEditDialog dlg(this);
    dlg.setEditMode(k);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    const QStringList values = dlg.keyValues();
    if (values.isEmpty()) {
        return;
    }

    k.setKeyValue(values.first());
    if (!repo.save(k)) {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось сохранить ключ:\n" + repo.lastError());
        return;
    }

    refresh();
}

void KeysTab::onDeleteClicked()
{
    const int id = selectedKeyId();
    if (id == 0) {
        QMessageBox::information(this, "Внимание",
            "Выберите ключ для удаления");
        return;
    }

    KeyRepository repo;
    ProductKey k = repo.getById(id);

    if (k.status() == ProductKey::Status::Sold) {
        QMessageBox::warning(this, "Нельзя удалить",
            "Этот ключ уже выдан клиенту. Удалять проданные ключи нельзя — "
            "это нарушит историю заказов.");
        return;
    }

    const auto answer = QMessageBox::question(this,
        "Подтверждение",
        QString("Удалить ключ \"%1\"?").arg(k.keyValue()),
        QMessageBox::Yes | QMessageBox::No);

    if (answer != QMessageBox::Yes) {
        return;
    }

    if (!repo.remove(id)) {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось удалить ключ:\n" + repo.lastError());
        return;
    }

    refresh();
}