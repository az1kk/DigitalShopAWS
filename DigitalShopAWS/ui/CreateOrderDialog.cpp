#include "CreateOrderDialog.h"

#include "../repositories/CustomerRepository.h"
#include "../repositories/ProductRepository.h"
#include "../repositories/KeyRepository.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

CreateOrderDialog::CreateOrderDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
    loadCustomers();
    loadProducts();
    updateProductInfo();
    updateOkEnabled();
}

void CreateOrderDialog::setupUi()
{
    setWindowTitle(QString::fromUtf8("Новый заказ"));
    setModal(true);
    resize(480, 280);

    m_customerCombo = new QComboBox(this);
    m_productCombo = new QComboBox(this);

    m_priceLabel = new QLabel("—", this);
    m_availableLabel = new QLabel("—", this);

    QFont infoFont = m_priceLabel->font();
    infoFont.setBold(true);
    m_priceLabel->setFont(infoFont);
    m_availableLabel->setFont(infoFont);

    QFormLayout* form = new QFormLayout;
    form->addRow(QString::fromUtf8("Клиент:"), m_customerCombo);
    form->addRow(QString::fromUtf8("Товар:"), m_productCombo);
    form->addRow(QString::fromUtf8("Цена:"), m_priceLabel);
    form->addRow(QString::fromUtf8("В наличии:"), m_availableLabel);

    m_okButton = new QPushButton(QString::fromUtf8("Создать заказ"), this);
    m_cancelButton = new QPushButton(QString::fromUtf8("Отмена"), this);
    m_okButton->setDefault(true);

    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(m_okButton);
    buttons->addWidget(m_cancelButton);

    QVBoxLayout* main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addStretch();
    main->addLayout(buttons);

    connect(m_customerCombo, &QComboBox::currentIndexChanged,
        this, &CreateOrderDialog::onCustomerChanged);
    connect(m_productCombo, &QComboBox::currentIndexChanged,
        this, &CreateOrderDialog::onProductChanged);

    connect(m_okButton, &QPushButton::clicked,
        this, &CreateOrderDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked,
        this, &CreateOrderDialog::reject);
}

void CreateOrderDialog::loadCustomers()
{
    m_customerCombo->blockSignals(true);
    m_customerCombo->clear();
    m_customerCombo->addItem(QString::fromUtf8("— выберите клиента —"), 0);

    CustomerRepository repo;
    for (const Customer& c : repo.getAll()) {
        QString label = c.name();
        if (!c.email().isEmpty()) {
            label += " (" + c.email() + ")";
        }
        m_customerCombo->addItem(label, c.id());
    }
    m_customerCombo->blockSignals(false);
}

void CreateOrderDialog::loadProducts()
{
    m_productCombo->blockSignals(true);
    m_productCombo->clear();
    m_productCombo->addItem(QString::fromUtf8("— выберите товар —"), 0);

    ProductRepository repo;
    for (const Product& p : repo.getAll()) {
        m_productCombo->addItem(p.name(), p.id());
    }
    m_productCombo->blockSignals(false);
}

void CreateOrderDialog::onCustomerChanged()
{
    updateOkEnabled();
}

void CreateOrderDialog::onProductChanged()
{
    updateProductInfo();
    updateOkEnabled();
}

void CreateOrderDialog::updateProductInfo()
{
    const int productId = selectedProductId();
    if (productId == 0) {
        m_priceLabel->setText("—");
        m_availableLabel->setText("—");
        return;
    }

    // Цена.
    ProductRepository prodRepo;
    Product p = prodRepo.getById(productId);
    m_priceLabel->setText(QString::number(p.price(), 'f', 2) + " " +
        QString::fromUtf8("₽"));

    // Доступные ключи.
    KeyRepository keyRepo;
    const int count = keyRepo.countAvailable(productId);
    if (count > 0) {
        m_availableLabel->setText(QString::number(count) + " " +
            QString::fromUtf8("шт."));
        m_availableLabel->setStyleSheet("color: #2E7D32;");  // зелёный
    }
    else {
        m_availableLabel->setText(QString::fromUtf8("нет в наличии"));
        m_availableLabel->setStyleSheet("color: #C62828;");  // красный
    }
}

void CreateOrderDialog::updateOkEnabled()
{
    const bool customerOk = selectedCustomerId() > 0;
    const bool productOk = selectedProductId() > 0;

    bool hasKeys = false;
    if (productOk) {
        KeyRepository repo;
        hasKeys = repo.countAvailable(selectedProductId()) > 0;
    }

    m_okButton->setEnabled(customerOk && productOk && hasKeys);
}

int CreateOrderDialog::selectedCustomerId() const
{
    return m_customerCombo->currentData().toInt();
}

int CreateOrderDialog::selectedProductId() const
{
    return m_productCombo->currentData().toInt();
}

void CreateOrderDialog::onOkClicked()
{
    if (selectedCustomerId() == 0 || selectedProductId() == 0) {
        return;
    }
    accept();
}