#pragma once

#include <QWidget>

class QTableWidget;
class QPushButton;

/**
 * @brief Вкладка управления товарами.
 *
 * Показывает таблицу всех товаров, позволяет добавлять, редактировать
 * и удалять записи. Использует ProductRepository для работы с БД.
 */
class ProductsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ProductsTab(QWidget* parent = nullptr);

public slots:
    /// Перезагружает данные из БД и обновляет таблицу.
    void refresh();

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();

private:
    void setupUi();

    /// Возвращает id товара, выбранного в таблице (0 если ничего не выбрано).
    int selectedProductId() const;

    QTableWidget* m_table = nullptr;
    QPushButton* m_addBtn = nullptr;
    QPushButton* m_editBtn = nullptr;
    QPushButton* m_deleteBtn = nullptr;
    QPushButton* m_refreshBtn = nullptr;
};