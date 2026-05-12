#include "ProductEditDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

ProductEditDialog::ProductEditDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
}

void ProductEditDialog::setupUi()
{
    setWindowTitle("Товар");
    setModal(true);
    resize(450, 400);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setMaxLength(200);

    m_priceSpin = new QDoubleSpinBox(this);
    m_priceSpin->setRange(0.0, 9999999.0);
    m_priceSpin->setDecimals(2);
    m_priceSpin->setSuffix(" ₽");
    m_priceSpin->setSingleStep(50.0);

    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setPlaceholderText("Описание товара (необязательно)");

    QFormLayout* form = new QFormLayout;
    form->addRow("Название:", m_nameEdit);
    form->addRow("Цена:", m_priceSpin);
    form->addRow("Описание:", m_descriptionEdit);

    m_okButton = new QPushButton("Сохранить", this);
    m_cancelButton = new QPushButton("Отмена", this);
    m_okButton->setDefault(true);

    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(m_okButton);
    buttons->addWidget(m_cancelButton);

    QVBoxLayout* main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addLayout(buttons);

    connect(m_okButton, &QPushButton::clicked,
        this, &ProductEditDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked,
        this, &ProductEditDialog::reject);
}

void ProductEditDialog::setProduct(const Product& product)
{
    m_editingId = product.id();
    m_nameEdit->setText(product.name());
    m_priceSpin->setValue(product.price());
    m_descriptionEdit->setPlainText(product.description());
}

Product ProductEditDialog::product() const
{
    Product p;
    p.setId(m_editingId);
    p.setName(m_nameEdit->text().trimmed());
    p.setPrice(m_priceSpin->value());
    p.setDescription(m_descriptionEdit->toPlainText().trimmed());
    return p;
}

void ProductEditDialog::onOkClicked()
{
    // Простая валидация.
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Проверка",
            "Введите название товара");
        m_nameEdit->setFocus();
        return;
    }

    if (m_priceSpin->value() <= 0.0) {
        QMessageBox::warning(this, "Проверка",
            "Цена должна быть больше нуля");
        m_priceSpin->setFocus();
        return;
    }

    accept();
}