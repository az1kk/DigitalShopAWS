#pragma once

#include <QWidget>

class QTableWidget;
class QPushButton;
class QComboBox;
class QCheckBox;

/**
 * @brief Вкладка управления складом ключей.
 *
 * В таблице показаны живые ключи: доступные и зарезервированные.
 * Проданные ключи по умолчанию скрыты — они относятся к истории
 * заказов и показываются на вкладке "Заказы" в колонке "Ключ".
 * При необходимости проданные можно показать чекбоксом
 * "Показать проданные".
 *
 * Сверху — фильтр по товару. При добавлении можно вставить сразу
 * пачку ключей.
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

    QComboBox*    m_productFilter = nullptr;
    QCheckBox*    m_showSoldCheck = nullptr;
    QTableWidget* m_table         = nullptr;
    QPushButton*  m_addBtn        = nullptr;
    QPushButton*  m_editBtn       = nullptr;
    QPushButton*  m_deleteBtn     = nullptr;
    QPushButton*  m_refreshBtn    = nullptr;
};
