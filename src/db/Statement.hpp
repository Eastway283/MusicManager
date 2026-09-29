#pragma once

// заголовочный файл, определяющий интерфейс подготовленных выражений(Statement)

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "db/Database.hpp"

// сокрытие определения sqlite3, нужен только указатель
struct sqlite3_stmt;

// класс подготовленных выражений - RAII обертка для SQLite API
class Statement {
    public:
        Statement(Database& db, std::string_view sql); // подготовка выражения для взаимодействия с БД
        ~Statement();                                  // деструктор для освобождения ресурсов

        // копирущий конструктор и оператор копирования запрещены
        // по причине уникальности дескриптора
        Statement(const Statement&) = delete;
        Statement& operator=(const Statement&) = delete;
        Statement(Statement&&) noexcept;
        Statement& operator=(Statement&&) noexcept;

        // функции для передачи значений в запрос к БД
        Statement& bind(int idx, int value);
        Statement& bind(int idx, std::int64_t value);
        Statement& bind(int idx, double value);
        Statement& bind(int idx, std::string_view value);
        Statement& bindNull(int idx);

        // true  - есть строка (SQLITE_ROW), читай через column*()
        // false - выполнено (SQLITE_DONE): для SELECT — строки кончились,
        //         для INSERT/UPDATE/DELETE — операция завершена
        // выполнение запроса на один шаг
        bool step();

        // сбросить стейтмент: возвращает в исходное состояние, снимает все привязки
        void reset();

        // получение значения по заданному индексу в строке
        int columnInt(int idx);
        std::int64_t columnInt64(int idx);
        double columnDouble(int idx);
        std::string columnText(int idx);
        std::optional<std::string> columnTextOpt(int idx);

        // проверка на NULL значение по заданному индексу
        bool isNull(int idx);

    private:
        sqlite3_stmt* stmt_ = nullptr;  // дескриптор подготовленного выражения
        sqlite3*      db_   = nullptr;  // дескриптор соединения с БД
};
