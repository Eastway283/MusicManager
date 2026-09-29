#pragma once

// данный заголовочный файл содержит объявление класса базы данных - обертки для SQLite API

#include <stdexcept>
#include <string>

// forward declaration - скрытие определения sqlite3, нужен только указатель
struct sqlite3;

// класс для ошибок времени выполнения
class DbError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
};

// класс базы данных - RAII обертка для безопасного использования
class Database {
    public:
        explicit Database(const std::string& path); // явный конструктор БД, с указанием имени
        ~Database();                                // деструктор

        // копирующий конструктор и оператор присваивания запрещены
        // по причине уникальности БД
        Database(const Database&) = delete;
        Database& operator=(const Database&) = delete;
        Database(Database&&) noexcept;
        Database& operator=(Database&&) noexcept;

        // exec - выполнение SQL-запроса без параметров
        void exec(const std::string& sql);

        // handle - возвращает сырой дескриптор БД для подготовленных выражений
        sqlite3* handle() const noexcept { return db_; }

    private:
        sqlite3* db_ = nullptr; // дескриптор соединения с БД, владение уникально
};
