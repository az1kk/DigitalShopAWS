#pragma once

#include <QDialog>
#include "../models/ProductKey.h"

class QComboBox;
class QPlainTextEdit;
class QLineEdit;
class QPushButton;
class QLabel;

/**
 * @brief Диалог добавления/редактирования ключей.
 *
 * Имеет два режима:
 *   1. Добавление новых ключей (можно сразу пачку).
 *      Используется текстовое поле для вставки списка строк —
 *      каждая строка превращается в отдельный ключ.
 *   2. Редактирование одного существующего ключа.
 *      Используется одна строка ввода.
 *
 * Режим задаётся методами addMode() / editMode(key).
 */
class KeyEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KeyEditDialog(QWidget* parent = nullptr);

    /// Переводит диалог в режим добавления (можно пачку ключей).
    void setAddMode();

    /// Переводит в режим редактирования одного ключа.
    void setEditMode(const ProductKey& key);

    /// Возвращает id выбранного товара.
    int selectedProductId() const;

    /// Список введённых ключей (одна строка = один ключ).
    /// В режиме редактирования всегда возвращает ровно один элемент.
    QStringList keyValues() const;

    /// В режиме редактирования — id редактируемого ключа.
    int editingKeyId() const { return m_editingId; }

private slots:
    void onOkClicked();

private:
    void setupUi();
    void loadProducts();

    QComboBox* m_productCombo = nullptr;
    QPlainTextEdit* m_keysMultiEdit = nullptr;   // для пачки
    QLineEdit* m_keySingleEdit = nullptr;   // для одного ключа при редактировании
    QLabel* m_keysHintLabel = nullptr;
    QPushButton* m_okButton = nullptr;
    QPushButton* m_cancelButton = nullptr;

    bool m_addMode = true;
    int  m_editingId = 0;
};