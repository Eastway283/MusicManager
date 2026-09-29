#include "db/Migration.hpp"
#include "db/Database.hpp"

void runMigration(Database &db) {
    // 1. Роли
     db.exec(R"(
        CREATE TABLE IF NOT EXISTS roles (
            id          INTEGER PRIMARY KEY,
            name        TEXT UNIQUE NOT NULL
        );
     )");
    // Пользователи - ссылается на roles
    db.exec(R"(
        CREATE TABLE IF NOT EXISTS users (
            id          INTEGER PRIMARY KEY,
            login       TEXT  UNIQUE NOT NULL,
            passwd_hash TEXT NOT NULL,
            salt        TEXT NOT NULL,
            role_id     INTEGER NOT NULL REFERENCES roles(id)
        );
    )");
    // Группы - ссылается на users
    db.exec(R"(
        CREATE TABLE IF NOT EXISTS bands (
            id          INTEGER PRIMARY KEY,
            name        TEXT NOT NULL,
            genre       TEXT NOT NULL,
            description TEXT,
            manager_id  INTEGER NOT NULL REFERENCES users(id)
        );
    )");
    // Музыканты - ссылается на bands
    db.exec(R"(
        CREATE TABLE IF NOT EXISTS musicians (
            id          INTEGER PRIMARY KEY,
            name        TEXT NOT NULL,
            phone       TEXT NOT NULL,
            email       TEXT,
            instrument  TEXT,
            band_id     INTEGER NOT NULL REFERENCES bands(id)
        );
    )");
    // Площадки - место проведение концертов
    db.exec(R"(
        CREATE TABLE IF NOT EXISTS venues (
            id          INTEGER PRIMARY KEY,
            name        TEXT NOT NULL,
            address     TEXT NOT NULL,
            capacity    INTEGER NOT NULL,
            contact     TEXT NOT NULL
        );
    )");
    // Концерты - ссылается на bands и venues
    db.exec(R"(
        CREATE TABLE IF NOT EXISTS concerts (
            id          INTEGER PRIMARY KEY,
            band_id     INTEGER NOT NULL REFERENCES bands(id),
            venue_id    INTEGER NOT NULL REFERENCES venues(id),
            date        TEXT NOT NULL,
            fee         REAL NOT NULL DEFAULT 0,
            expenses    REAL NOT NULL DEFAULT 0,
            status      TEXT NOT NULL DEFAULT 'planned'
        );
    )");
    // Договоры - ссылается на concerts
    db.exec(R"(
        CREATE TABLE IF NOT EXISTS contracts (
            id          INTEGER PRIMARY KEY,
            concert_id  INTEGER NOT NULL REFERENCES concerts(id),
            number      TEXT NOT NULL,
            date        TEXT NOT NULL,
            amount      REAL NOT NULL,
            status      TEXT NOT NULL DEFAULT 'draft'
        );
    )");

    // Создание базовых ролей
    db.exec(R"(INSERT OR IGNORE INTO roles(name) VALUES ('admin'))");
    db.exec(R"(INSERT OR IGNORE INTO roles(name) VALUES ('manager'))");
}
