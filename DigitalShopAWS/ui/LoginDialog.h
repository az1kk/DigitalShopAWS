#pragma once

#include <QDialog>

class QLineEdit;
class QPushButton;
class QLabel;

/**
 * @brief Диалог входа в систему.
 *
 * Показывается перед главным окном. Запрашивает логин и пароль,
 * проверяет их через UserRepository + PasswordHasher.
 *
 * Использование:
 *   LoginDialog dlg;
 *   if (dlg.exec() == QDialog::Accepted) {
 *       int userId = dlg.authenticatedUserId();
 *       // открываем главное окно
 *   } else {
 *       // отмена → выход из приложения
 *   }
 */
class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);

    /// id успешно вошедшего пользователя. 0, если входа не было.
    int authenticatedUserId() const { return m_userId; }

private slots:
    /// Срабатывает при нажатии "Войти" или Enter в поле пароля.
    void onLoginClicked();

private:
    void setupUi();

    QLineEdit* m_loginEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QPushButton* m_loginButton = nullptr;
    QPushButton* m_cancelButton = nullptr;
    QLabel* m_errorLabel = nullptr;

    int m_userId = 0;
};