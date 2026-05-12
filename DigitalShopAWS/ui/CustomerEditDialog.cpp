#include "CustomerEditDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

CustomerEditDialog::CustomerEditDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
}

void CustomerEditDialog::setupUi()
{
    setWindowTitle("Клиент");
    setModal(true);
    resize(420, 220);

    m_nameEdit = new QLineEdit(this);
    m_emailEdit = new QLineEdit(this);
    m_phoneEdit = new QLineEdit(this);

    m_nameEdit->setMaxLength(150);
    m_emailEdit->setMaxLength(150);
    m_emailEdit->setPlaceholderText("client@example.com");
    m_phoneEdit->setMaxLength(30);
    m_phoneEdit->setPlaceholderText("+7 999 123-45-67");

    QFormLayout* form = new QFormLayout;
    form->addRow("Имя:", m_nameEdit);
    form->addRow("Email:", m_emailEdit);
    form->addRow("Телефон:", m_phoneEdit);

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
        this, &CustomerEditDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked,
        this, &CustomerEditDialog::reject);
}

void CustomerEditDialog::setCustomer(const Customer& customer)
{
    m_editingId = customer.id();
    m_nameEdit->setText(customer.name());
    m_emailEdit->setText(customer.email());
    m_phoneEdit->setText(customer.phone());
}

Customer CustomerEditDialog::customer() const
{
    Customer c;
    c.setId(m_editingId);
    c.setName(m_nameEdit->text().trimmed());
    c.setEmail(m_emailEdit->text().trimmed());
    c.setPhone(m_phoneEdit->text().trimmed());
    return c;
}

void CustomerEditDialog::onOkClicked()
{
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Проверка",
            "Введите имя клиента");
        m_nameEdit->setFocus();
        return;
    }

    accept();
}