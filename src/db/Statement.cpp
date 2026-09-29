#include "db/Statement.hpp"
#include "db/Database.hpp"

#include <optional>
#include <sqlite3.h>
#include <string>
#include <string_view>

// конструктор подготовленного выражения, создает выражение, привязанное к db_
// throw DbError в случае ошибки
Statement::Statement(Database& db, std::string_view sql) : db_(db.handle()) {
    if (sqlite3_prepare_v2(db_, sql.data(), static_cast<int>(sql.size()), &stmt_, nullptr) != SQLITE_OK)
            throw DbError(std::string("Prepare failed: ") + sqlite3_errmsg(db_)
                + "\nQuery: " + std::string(sql));
}

// уничтожение подготовленного выражения
Statement::~Statement() {
    if (stmt_)
        sqlite3_finalize(stmt_);
}

// перемещающий конструктор и оператор перемещения
// обнуляют other подготовленное выражение
Statement::Statement(Statement&& other) noexcept : stmt_(other.stmt_), db_(other.db_) {
    other.stmt_ = nullptr;
    other.db_   = nullptr;
}

Statement& Statement::operator=(Statement&& other) noexcept {
    if (this != &other) {
        if (stmt_)
            sqlite3_finalize(stmt_);
        stmt_ = other.stmt_;
        db_ = other.db_;
        other.stmt_ = nullptr;
        other.db_   = nullptr;
    }
    return *this;
}

// методы для передачи значений в запрос
// возвращают указатель на Statement
Statement& Statement::bind(int idx, int value) {
    sqlite3_bind_int(stmt_, idx, value);
    return *this;
}

Statement& Statement::bind(int idx, std::int64_t value) {
    sqlite3_bind_int64(stmt_, idx, value);
    return *this;
}

Statement& Statement::bind(int idx, double value) {
    sqlite3_bind_double(stmt_, idx, value);
    return *this;
}

Statement& Statement::bind(int idx, std::string_view value) {
    sqlite3_bind_text(stmt_, idx, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT);
    return *this;
}

Statement& Statement::bindNull(int idx) {
    sqlite3_bind_null(stmt_, idx);
    return *this;
}

// step() - выполняет запрос на один шаг
// true  - есть строка (SQLITE_ROW)
// false - выполнено (SQLITE_DONE)
// throw DbError при прочих кодах
bool Statement::step() {
    int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW)
        return true;
    if (rc == SQLITE_DONE)
        return false;
    throw DbError(std::string("Step failed: ") + sqlite3_errmsg(db_));
}

// reset() - сбрасывает параметры подготовленного выражения
// снимает все привязки
void Statement::reset() {
    sqlite3_reset(stmt_);
    sqlite3_clear_bindings(stmt_);
}

// column*(int idx) - возвращает значение ячейки с индексом idx
int Statement::columnInt(int idx) {
    return sqlite3_column_int(stmt_, idx);
}

std::int64_t Statement::columnInt64(int idx) {
    return sqlite3_column_int64(stmt_, idx);
}

double Statement::columnDouble(int idx) {
    return sqlite3_column_double(stmt_, idx);
}

std::string Statement::columnText(int idx) {
    const unsigned char* p = sqlite3_column_text(stmt_, idx);
    return p ? reinterpret_cast<const char *>(p) : std::string{};
}

std::optional<std::string> Statement::columnTextOpt(int idx) {
    if (sqlite3_column_type(stmt_, idx) == SQLITE_NULL)
        return std::nullopt;
    return columnText(idx);
}

// isNull - возвращает результат проверки типа ячейки с индексом idx с NULL
bool Statement::isNull(int idx) {
    return sqlite3_column_type(stmt_, idx) == SQLITE_NULL;
}
