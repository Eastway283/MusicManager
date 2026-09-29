#include "db/Database.hpp"
#include <sqlite3.h>

// создает/открывает БД, включает внешние ключи
// throw DbError - в случае ошибки
Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        std::string msg = db_ ? sqlite3_errmsg(db_) : "unknown error";
        sqlite3_close(db_);
        db_ = nullptr;
        throw DbError("Cannot open database '" + path + "': " + msg);
    }
    // Включаем внешние ключи - по умолчанию SQLite их игнорирует
    sqlite3_exec(db_, "PRAGMA foreign_keys = ON", nullptr, nullptr, nullptr);
}

// закрывает соединение при уничтожении объекта
Database::~Database() {
    if (db_)
        sqlite3_close(db_);
}

// перемещающий конструктор и оператор присваивания - перемещают и обнуляют other.db_
Database::Database(Database&& other) noexcept : db_(other.db_) {
    other.db_ = nullptr;
}

Database& Database::operator=(Database&& other) noexcept {
    if (this != &other) {
        if (db_)
            sqlite3_close(db_);
        db_ = other.db_;
        other.db_ = nullptr;
    }
    return *this;
}

// выполнение SQL-запроса без параметров, throw DbError в случае ошибки
void Database::exec(const std::string& sql) {
    char *err = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown error";
        sqlite3_free(err);
        throw DbError("SQL error: " + msg + "\nQuery: " + sql);
    }
}
