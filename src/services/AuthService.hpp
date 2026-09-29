#pragma once

#include "Models.hpp"

// Данный файл содержит заголовочный файл сервиса аунтефикации пользователей в системе

#include <optional>
#include <string>
#include <vector>

class Database;

class AuthService {
    public:
    explicit AuthService(Database& db);

    // Создает пользователя. Возвращает id созданного пользователя
    // Бросает DbError при ошибке БД
    int createUser(const std::string& login, const std::string& passwd, int role_id);

    // Ищет пользователя по логину. Если пароль совпал - возвращает User
    // Если логина нет или пароль неверный - nullopt
    std::optional<models::User> login(const std::string& login, const std::string& passwd);

    // Возвращает список всех пользователей в БД
    // в БД всегда есть как минимум 1 администратор
    std::vector<models::User> getAll() const;

    // Возвращает список ролей в БД
    std::vector<models::Role> getAllRoles() const;

    // Удаляет пользователя из БД по его id
    // если id не валиден - ничего не делает
    void remove(int id);

    // Проверяет, есть ли вообще пользователи в БД
    bool hasAnyUser() const;

    private:
    Database& db_;
};
