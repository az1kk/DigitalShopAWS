#include "LoginDialog.h"

#include "../repositories/UserRepository.h"
#include "../utils/PasswordHasher.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
}

void LoginDialog::setupUi()
{
    setWindowTitle("Вход в систему");
    setModal(true);
    setFixedSize(360, 200);

    // Поля ввода
    m_loginEdit = new QLineEdit(this);
    m_loginEdit->setPlaceholderText("admin");

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("••••••");

    // Форма с подписями
    QFormLayout* form = new QFormLayout;
    form->addRow("Логин:", m_loginEdit);
    form->addRow("Пароль:", m_passwordEdit);

    // Метка ошибки (скрыта по умолчанию)
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: red;");
    m_errorLabel->setVisible(false);

    // Кнопки
    m_loginButton = new QPushButton("Войти", this);
    m_cancelButton = new QPushButton("Отмена", this);
    m_loginButton->setDefault(true);

    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(m_loginButton);
    buttons->addWidget(m_cancelButton);

    // Главный вертикальный layout
    QVBoxLayout* main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addWidget(m_errorLabel);
    main->addStretch();
    main->addLayout(buttons);

    // Связи сигналов и слотов.
    connect(m_loginButton, &QPushButton::clicked,
        this, &LoginDialog::onLoginClicked);
    connect(m_cancelButton, &QPushButton::clicked,
        this, &LoginDialog::reject);

    // Enter в поле пароля = нажатие "Войти".
    connect(m_passwordEdit, &QLineEdit::returnPressed,
        this, &LoginDialog::onLoginClicked);
}

void LoginDialog::onLoginClicked()
{
    const QString login = m_loginEdit->text().trimmed();
    const QString password = m_passwordEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        m_errorLabel->setText("Введите логин и пароль");
        m_errorLabel->setVisible(true);
        return;
    }

    UserRepository repo;
    User user = repo.getByLogin(login);

    if (!user.isValid()) {
        m_errorLabel->setText("Пользователь не найден");
        m_errorLabel->setVisible(true);
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
        return;
    }

    if (!PasswordHasher::verify(password, user.passwordHash())) {
        m_errorLabel->setText("Неверный пароль");
        m_errorLabel->setVisible(true);
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
        return;
    }

    // Успех.
    m_userId = user.id();
    accept();
}