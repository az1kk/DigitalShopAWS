#include "KeyEditDialog.h"

#include "../repositories/ProductRepository.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

KeyEditDialog::KeyEditDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
    loadProducts();
    setAddMode();
}

void KeyEditDialog::setupUi()
{
    setModal(true);
    resize(520, 420);

    m_productCombo = new QComboBox(this);

    m_keysMultiEdit = new QPlainTextEdit(this);
    m_keysMultiEdit->setPlaceholderText(
        "AAAA-BBBB-CCCC-1111\n"
        "AAAA-BBBB-CCCC-2222\n"
        "AAAA-BBBB-CCCC-3333\n"
        "...");

    m_keySingleEdit = new QLineEdit(this);
    m_keySingleEdit->setMaxLength(200);

    m_keysHintLabel = new QLabel(
        "<i>Каждая строка — один ключ. Пустые строки игнорируются.</i>", this);
    m_keysHintLabel->setStyleSheet("color: gray;");

    QFormLayout* form = new QFormLayout;
    form->addRow("Товар:", m_productCombo);
    form->addRow("Ключи:", m_keysMultiEdit);
    form->addRow("", m_keysHintLabel);
    form->addRow("Ключ:", m_keySingleEdit);

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

    connect(m_okButton, &QPushButton::clicked, this, &KeyEditDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &KeyEditDialog::reject);
}

void KeyEditDialog::loadProducts()
{
    m_productCombo->clear();

    ProductRepository repo;
    const auto products = repo.getAll();

    for (const Product& p : products) {
        // В тексте — название, в данных — id товара.
        m_productCombo->addItem(p.name(), p.id());
    }
}

void KeyEditDialog::setAddMode()
{
    m_addMode = true;
    m_editingId = 0;
    setWindowTitle("Добавление ключей");

    m_keysMultiEdit->setVisible(true);
    m_keysHintLabel->setVisible(true);
    m_keySingleEdit->setVisible(false);

    m_productCombo->setEnabled(true);
    m_keysMultiEdit->clear();
}

void KeyEditDialog::setEditMode(const ProductKey& key)
{
    m_addMode = false;
    m_editingId = key.id();
    setWindowTitle("Редактирование ключа");

    m_keysMultiEdit->setVisible(false);
    m_keysHintLabel->setVisible(false);
    m_keySingleEdit->setVisible(true);

    m_keySingleEdit->setText(key.keyValue());

    // Выбираем нужный товар в комбобоксе.
    for (int i = 0; i < m_productCombo->count(); ++i) {
        if (m_productCombo->itemData(i).toInt() == key.productId()) {
            m_productCombo->setCurrentIndex(i);
            break;
        }
    }
    // Менять товар при редактировании ключа нет смысла.
    m_productCombo->setEnabled(false);
}

int KeyEditDialog::selectedProductId() const
{
    return m_productCombo->currentData().toInt();
}

QStringList KeyEditDialog::keyValues() const
{
    QStringList result;

    if (m_addMode) {
        const QStringList lines = m_keysMultiEdit->toPlainText().split('\n', Qt::SkipEmptyParts);
        for (const QString& line : lines) {
            const QString trimmed = line.trimmed();
            if (!trimmed.isEmpty()) {
                result.append(trimmed);
            }
        }
    }
    else {
        const QString trimmed = m_keySingleEdit->text().trimmed();
        if (!trimmed.isEmpty()) {
            result.append(trimmed);
        }
    }

    return result;
}

void KeyEditDialog::onOkClicked()
{
    if (m_productCombo->count() == 0) {
        QMessageBox::warning(this, "Проверка",
            "Сначала добавьте хотя бы один товар на вкладке \"Товары\"");
        return;
    }

    if (keyValues().isEmpty()) {
        QMessageBox::warning(this, "Проверка",
            "Введите хотя бы один ключ");
        return;
    }

    accept();
}