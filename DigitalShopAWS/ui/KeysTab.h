#pragma once

#include <QWidget>

class QTableWidget;
class QPushButton;
class QComboBox;

/**
 * @brief Вкладка управления складом ключей.
 *
 * В таблице показаны все ключи: id, товар, значение, статус, дата добавления.
 * Сверху — фильтр по товару, позволяющий показать ключи только одного товара.
 * При добавлении можно вставить сразу пачку ключей.
 */
class KeysTab : public QWidget
{
    Q_OBJECT

public:
    explicit KeysTab(QWidget* parent = nullptr);

public slots:
    void refresh();

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onFilterChanged();

private:
    void setupUi();
    void reloadProductFilter();
    int selectedKeyId() const;

    QComboBox* m_productFilter = nullptr;
    QTableWidget* m_table = nullptr;
    QPushButton* m_addBtn = nullptr;
    QPushButton* m_editBtn = nullptr;
    QPushButton* m_deleteBtn = nullptr;
    QPushButton* m_refreshBtn = nullptr;
};