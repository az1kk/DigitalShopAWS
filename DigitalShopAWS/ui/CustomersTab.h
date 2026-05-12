#pragma once

#include <QWidget>

class QTableWidget;
class QPushButton;

/**
 * @brief Вкладка управления клиентами.
 *
 * Структура и логика идентичны ProductsTab, отличаются только поля.
 */
class CustomersTab : public QWidget
{
    Q_OBJECT

public:
    explicit CustomersTab(QWidget* parent = nullptr);

public slots:
    void refresh();

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();

private:
    void setupUi();
    int selectedCustomerId() const;

    QTableWidget* m_table = nullptr;
    QPushButton* m_addBtn = nullptr;
    QPushButton* m_editBtn = nullptr;
    QPushButton* m_deleteBtn = nullptr;
    QPushButton* m_refreshBtn = nullptr;
};