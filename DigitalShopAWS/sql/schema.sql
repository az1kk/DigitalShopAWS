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
-- Свободные ключи (для демонстрации работы программы — создания новых заказов)
INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status)
VALUES (1, 1, 'STEAM-AAAA-1111-BBBB-2222', 'available');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status)
VALUES (2, 1, 'STEAM-CCCC-3333-DDDD-4444', 'available');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status)
VALUES (3, 2, 'MC-JAVA-XK7P-Q9MN-RT3V', 'available');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status)
VALUES (4, 3, 'NETFLIX-USR-2026-MAY-001', 'available');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status)
VALUES (5, 4, 'SPOT-PREM-MAY26-AB12CD', 'available');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status)
VALUES (6, 5, 'OFFICE365-PERS-2026-XYZW-1234', 'available');

-- Проданные ключи (привязаны к тестовым заказам ниже)
INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status, sold_at)
VALUES (7, 2, 'MC-JAVA-SOLD-AAA1-2025-12', 'sold', '2025-12-10 14:32:00');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status, sold_at)
VALUES (8, 3, 'NETFLIX-SOLD-2026-JAN-001', 'sold', '2026-01-08 11:15:00');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status, sold_at)
VALUES (9, 5, 'OFFICE365-SOLD-2026-JAN-XX', 'sold', '2026-01-22 09:48:00');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status, sold_at)
VALUES (10, 1, 'STEAM-SOLD-2026-FEB-AAA1', 'sold', '2026-02-05 18:22:00');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status, sold_at)
VALUES (11, 1, 'STEAM-SOLD-2026-FEB-BBB2', 'sold', '2026-02-18 20:10:00');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status, sold_at)
VALUES (12, 4, 'SPOT-SOLD-2026-MAR-CCC3', 'sold', '2026-03-12 13:05:00');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status, sold_at)
VALUES (13, 2, 'MC-JAVA-SOLD-2026-MAR-DD', 'sold', '2026-03-27 16:40:00');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status, sold_at)
VALUES (14, 3, 'NETFLIX-SOLD-2026-APR-001', 'sold', '2026-04-09 10:30:00');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status, sold_at)
VALUES (15, 5, 'OFFICE365-SOLD-2026-APR-YY', 'sold', '2026-04-19 15:12:00');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status, sold_at)
VALUES (16, 2, 'MC-JAVA-SOLD-2026-MAY-EE5', 'sold', '2026-05-03 12:45:00');

INSERT OR IGNORE INTO product_keys (id, product_id, key_value, status, sold_at)
VALUES (17, 1, 'STEAM-SOLD-2026-MAY-FFF6', 'sold', '2026-05-08 19:20:00');

-- Тестовые заказы (Paid) — для демонстрации аналитики продаж по месяцам
INSERT OR IGNORE INTO orders (id, customer_id, product_id, key_id, price, status, created_at, paid_at)
VALUES (1, 1, 2, 7, 1990.00, 'paid', '2025-12-10 14:30:00', '2025-12-10 14:32:00');

INSERT OR IGNORE INTO orders (id, customer_id, product_id, key_id, price, status, created_at, paid_at)
VALUES (2, 2, 3, 8, 899.00, 'paid', '2026-01-08 11:10:00', '2026-01-08 11:15:00');

INSERT OR IGNORE INTO orders (id, customer_id, product_id, key_id, price, status, created_at, paid_at)
VALUES (3, 3, 5, 9, 3490.00, 'paid', '2026-01-22 09:45:00', '2026-01-22 09:48:00');

INSERT OR IGNORE INTO orders (id, customer_id, product_id, key_id, price, status, created_at, paid_at)
VALUES (4, 1, 1, 10, 199.00, 'paid', '2026-02-05 18:20:00', '2026-02-05 18:22:00');

INSERT OR IGNORE INTO orders (id, customer_id, product_id, key_id, price, status, created_at, paid_at)
VALUES (5, 2, 1, 11, 199.00, 'paid', '2026-02-18 20:08:00', '2026-02-18 20:10:00');

INSERT OR IGNORE INTO orders (id, customer_id, product_id, key_id, price, status, created_at, paid_at)
VALUES (6, 3, 4, 12, 299.00, 'paid', '2026-03-12 13:02:00', '2026-03-12 13:05:00');

INSERT OR IGNORE INTO orders (id, customer_id, product_id, key_id, price, status, created_at, paid_at)
VALUES (7, 1, 2, 13, 1990.00, 'paid', '2026-03-27 16:38:00', '2026-03-27 16:40:00');

INSERT OR IGNORE INTO orders (id, customer_id, product_id, key_id, price, status, created_at, paid_at)
VALUES (8, 2, 3, 14, 899.00, 'paid', '2026-04-09 10:28:00', '2026-04-09 10:30:00');

INSERT OR IGNORE INTO orders (id, customer_id, product_id, key_id, price, status, created_at, paid_at)
VALUES (9, 3, 5, 15, 3490.00, 'paid', '2026-04-19 15:10:00', '2026-04-19 15:12:00');

INSERT OR IGNORE INTO orders (id, customer_id, product_id, key_id, price, status, created_at, paid_at)
VALUES (10, 1, 2, 16, 1990.00, 'paid', '2026-05-03 12:43:00', '2026-05-03 12:45:00');

INSERT OR IGNORE INTO orders (id, customer_id, product_id, key_id, price, status, created_at, paid_at)
VALUES (11, 2, 1, 17, 199.00, 'paid', '2026-05-08 19:18:00', '2026-05-08 19:20:00');