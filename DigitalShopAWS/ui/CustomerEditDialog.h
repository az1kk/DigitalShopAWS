#pragma once

#include <QDialog>
#include "../models/Customer.h"

class QLineEdit;
class QPushButton;

/**
 * @brief Диалог добавления/редактирования клиента.
 *
 * Аналогичен ProductEditDialog по структуре:
 *   - пустой конструктор для добавления нового;
 *   - setCustomer() для заполнения формы при редактировании;
 *   - customer() для получения данных после нажатия "Сохранить".
 */
class CustomerEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CustomerEditDialog(QWidget* parent = nullptr);

    void setCustomer(const Customer& customer);
    Customer customer() const;

private slots:
    void onOkClicked();

private:
    void setupUi();

    QLineEdit* m_nameEdit = nullptr;
    QLineEdit* m_emailEdit = nullptr;
    QLineEdit* m_phoneEdit = nullptr;
    QPushButton* m_okButton = nullptr;
    QPushButton* m_cancelButton = nullptr;

    int m_editingId = 0;
};