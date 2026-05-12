#pragma once

#include <QDialog>
#include "../models/Product.h"

class QLineEdit;
class QDoubleSpinBox;
class QTextEdit;
class QPushButton;

/**
 * @brief Диалог добавления/редактирования товара.
 *
 * Использование для нового товара:
 *   ProductEditDialog dlg;
 *   if (dlg.exec() == QDialog::Accepted) {
 *       Product p = dlg.product();
 *       // сохраняем p
 *   }
 *
 * Использование для редактирования:
 *   ProductEditDialog dlg;
 *   dlg.setProduct(existingProduct);
 *   if (dlg.exec() == QDialog::Accepted) {
 *       Product p = dlg.product();
 *       // сохраняем p
 *   }
 */
class ProductEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProductEditDialog(QWidget* parent = nullptr);

    /// Заполняет поля формы значениями из товара (для редактирования).
    void setProduct(const Product& product);

    /// Собирает данные из полей формы в объект Product.
    Product product() const;

private slots:
    void onOkClicked();

private:
    void setupUi();

    QLineEdit* m_nameEdit = nullptr;
    QDoubleSpinBox* m_priceSpin = nullptr;
    QTextEdit* m_descriptionEdit = nullptr;
    QPushButton* m_okButton = nullptr;
    QPushButton* m_cancelButton = nullptr;

    // Сохраняем id редактируемого товара (0 если новый).
    int m_editingId = 0;
};