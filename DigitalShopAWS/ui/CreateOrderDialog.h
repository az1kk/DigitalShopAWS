#pragma once

#include <QDialog>

class QComboBox;
class QLabel;
class QPushButton;

/**
 * @brief Диалог создания заказа.
 *
 * Оператор выбирает клиента и товар. Диалог показывает текущую цену
 * товара и количество доступных ключей. Кнопка "Создать" активна только
 * если выбраны и клиент, и товар, и есть хотя бы один свободный ключ.
 */
class CreateOrderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateOrderDialog(QWidget* parent = nullptr);

    int selectedCustomerId() const;
    int selectedProductId() const;

private slots:
    void onProductChanged();
    void onCustomerChanged();
    void onOkClicked();

private:
    void setupUi();
    void loadCustomers();
    void loadProducts();
    void updateProductInfo();
    void updateOkEnabled();

    QComboBox* m_customerCombo = nullptr;
    QComboBox* m_productCombo = nullptr;
    QLabel* m_priceLabel = nullptr;
    QLabel* m_availableLabel = nullptr;
    QPushButton* m_okButton = nullptr;
    QPushButton* m_cancelButton = nullptr;
};