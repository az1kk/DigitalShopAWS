-- Схема базы данных DigitalShopAWS
-- Все таблицы создаются с IF NOT EXISTS, чтобы скрипт можно было выполнять
-- многократно без ошибок при существующих таблицах.

CREATE TABLE IF NOT EXISTS users (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    login         TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    role          TEXT NOT NULL DEFAULT 'admin',
    created_at    DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS categories (
    id   INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS products (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    category_id INTEGER REFERENCES categories(id),
    name        TEXT NOT NULL,
    description TEXT,
    price       REAL NOT NULL,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS product_keys (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    product_id INTEGER NOT NULL REFERENCES products(id),
    key_value  TEXT NOT NULL UNIQUE,
    status     TEXT NOT NULL DEFAULT 'available',
    added_at   DATETIME DEFAULT CURRENT_TIMESTAMP,
    sold_at    DATETIME
);

CREATE TABLE IF NOT EXISTS customers (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    name       TEXT NOT NULL,
    email      TEXT UNIQUE,
    phone      TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS orders (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    customer_id INTEGER REFERENCES customers(id),
    product_id  INTEGER REFERENCES products(id),
    key_id      INTEGER REFERENCES product_keys(id),
    price       REAL NOT NULL,
    status      TEXT NOT NULL DEFAULT 'pending',
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
    paid_at     DATETIME
);

-- Создаём дефолтного администратора при первом запуске.
-- Логин: admin, пароль: admin (хеш SHA-256 от строки "admin")
INSERT OR IGNORE INTO users (login, password_hash, role)
VALUES ('admin', '8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918', 'admin');

-- Тестовые категории
INSERT OR IGNORE INTO categories (id, name) VALUES (1, 'Игровые ключи');
INSERT OR IGNORE INTO categories (id, name) VALUES (2, 'Подписки');
INSERT OR IGNORE INTO categories (id, name) VALUES (3, 'Программное обеспечение');

-- Тестовые товары
INSERT OR IGNORE INTO products (id, category_id, name, description, price)
VALUES (1, 1, 'Steam Random Key', 'Случайный ключ Steam игры стоимостью от 500 руб', 199.00);

INSERT OR IGNORE INTO products (id, category_id, name, description, price)
VALUES (2, 1, 'Minecraft Java Edition', 'Лицензионный ключ Minecraft Java Edition', 1990.00);

INSERT OR IGNORE INTO products (id, category_id, name, description, price)
VALUES (3, 2, 'Netflix Premium 1 месяц', 'Подписка Netflix Premium на 1 месяц, 4K качество', 899.00);

INSERT OR IGNORE INTO products (id, category_id, name, description, price)
VALUES (4, 2, 'Spotify Premium 1 месяц', 'Индивидуальная подписка Spotify Premium на 1 месяц', 299.00);

INSERT OR IGNORE INTO products (id, category_id, name, description, price)
VALUES (5, 3, 'Office 365 Personal', 'Лицензия Microsoft Office 365 на 1 год', 3490.00);

-- Тестовые клиенты
INSERT OR IGNORE INTO customers (id, name, email, phone)
VALUES (1, 'Иванов Иван Иванович', 'ivanov@mail.ru', '+7 912 345-67-89');

INSERT OR IGNORE INTO customers (id, name, email, phone)
VALUES (2, 'Петрова Анна Сергеевна', 'petrova@gmail.com', '+7 999 876-54-32');

INSERT OR IGNORE INTO customers (id, name, email, phone)
VALUES (3, 'Сидоров Алексей Викторович', 'sidorov@yandex.ru', '+7 905 111-22-33');

-- Тестовые ключи для товаров
INSERT OR IGNORE INTO product_keys (product_id, key_value, status)
VALUES (1, 'STEAM-AAAA-1111-BBBB-2222', 'available');

INSERT OR IGNORE INTO product_keys (product_id, key_value, status)
VALUES (1, 'STEAM-CCCC-3333-DDDD-4444', 'available');

INSERT OR IGNORE INTO product_keys (product_id, key_value, status)
VALUES (1, 'STEAM-EEEE-5555-FFFF-6666', 'available');

INSERT OR IGNORE INTO product_keys (product_id, key_value, status)
VALUES (2, 'MC-JAVA-XK7P-Q9MN-RT3V', 'available');

INSERT OR IGNORE INTO product_keys (product_id, key_value, status)
VALUES (2, 'MC-JAVA-LP4D-W8YH-Z2BC', 'available');

INSERT OR IGNORE INTO product_keys (product_id, key_value, status)
VALUES (3, 'NETFLIX-USR-2026-MAY-001', 'available');

INSERT OR IGNORE INTO product_keys (product_id, key_value, status)
VALUES (3, 'NETFLIX-USR-2026-MAY-002', 'available');

INSERT OR IGNORE INTO product_keys (product_id, key_value, status)
VALUES (4, 'SPOT-PREM-MAY26-AB12CD', 'available');

INSERT OR IGNORE INTO product_keys (product_id, key_value, status)
VALUES (4, 'SPOT-PREM-MAY26-EF34GH', 'available');

INSERT OR IGNORE INTO product_keys (product_id, key_value, status)
VALUES (5, 'OFFICE365-PERS-2026-XYZW-1234', 'available');