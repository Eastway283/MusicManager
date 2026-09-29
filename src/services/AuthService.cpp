#include "services/AuthService.hpp"
#include "Models.hpp"
#include "db/Database.hpp"
#include "db/Statement.hpp"
#include "sqlite3.h"
#include "util/Hash.hpp"

#include <string>
#include <optional>

static models::User rowToUser(Statement& s) {
    models::User u;
    u.id          = s.columnInt(0);
    u.login       = s.columnText(1);
    u.passwd_hash = s.columnText(2);
    u.salt        = s.columnText(3);
    u.role_id     = s.columnInt(4);
    return u;
}

// Конструктор: сохраняет ссылку на БД. Сервис не владеет базой,
// только использует её. Поэтому в поле хранится Database&, а не копия
AuthService::AuthService(Database& db) : db_{db} {}


// Создает нового пользователя: генерирует соль, считает SHA-256,
// вставляет запись в таблицу user
// Возвращает id только что созданного пользователя
// Бросает DbError в случае ошибки БД
int AuthService::createUser(const std::string& login, const std::string& passwd, int role_id) {
    std::string salt = generateSalt();
    std::string hash = hashPasswd(passwd, salt);
    Statement s(db_, "INSERT INTO users (login, passwd_hash, salt, role_id) "
                     "VALUES (?, ?, ?, ?)");
    s.bind(1, login).bind(2, hash).bind(3, salt).bind(4, role_id);
    s.step();

    // id последней вставленной строки для этого соединения
    return static_cast<int>(sqlite3_last_insert_rowid(db_.handle()));
}

// Проверяет логин и пароль.
// Возвращает User, если пользователь существует в БД И пароль совпал с сохраненным хешем
// Возвращает nullopt, если логина нет ИЛИ пароль не совпал
// Бросает DbError при ошибке БД
//
// Сравнение: считаем hashPasswd(введенный пароль, соль из БД) и
// сравниваем с сохраненным хешем пароль из БД. Соль берется из записи, не генерируется заново
std::optional<models::User> AuthService::login(const std::string& login, const std::string& passwd) {
    Statement s(db_, "SELECT id, login, passwd_hash, salt, role_id " 
                     "FROM users WHERE login = ?"); // Формирование запроса к БД
    s.bind(1, login);

    if(!s.step())
        return std::nullopt; // Пользователя с таким логином нет

    // Заполнение полей пользователя из БД
    models::User user = rowToUser(s);

    // Вычисление хеша введенного пароля и соли пользователя
    std::string computed = hashPasswd(passwd, user.salt);
    if (computed != user.passwd_hash)
        return std::nullopt; // хеш пароля из БД не совпал с хешем введенного - отказ

    return user;
}

// Получение списка всех пользователей, которые есть в БД
std::vector<models::User> AuthService::getAll() const {
    std::vector<models::User> res;
    Statement s(db_, "SELECT id, login, passwd_hash, salt, role_id "
                     "FROM users ORDER BY login");
    while (s.step())
        res.push_back(rowToUser(s));
    return res;
}

// Получение списка всех ролей, которые есть в БД
std::vector<models::Role> AuthService::getAllRoles() const {
    std::vector<models::Role> res;
    Statement s(db_, "SELECT id, name FROM roles ORDER BY id");
    while (s.step()) {
        models::Role r;
        r.id   = s.columnInt(0);
        r.name = s.columnText(1);
        res.push_back(std::move(r));
    }
    return res;
}

// Удаление пользователя из БД по его id
// если id не валиден - ничего не делает
void AuthService::remove(int id) {
    Statement s(db_, "DELETE FROM users WHERE id = ?");
    s.bind(1, id);
    s.step();
}

// Проверяет наличие в БД ХОТЯ БЫ одного пользователя
// true - если таковой есть, false - в противном случае
bool AuthService::hasAnyUser() const {
    Statement s(db_, "SELECT 1 FROM users LIMIT 1");
    return s.step();
}
