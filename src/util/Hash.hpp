#pragma once

// Данный заголовочный файл содержит объявление методов для хэширования паролей пользователей
// является частью системы защиты от взломов

#include <string>

// generateSalt() - возвращает hex-строку из 16 случайных байт
std::string generateSalt();

// hashPasswd(passwd, salt) - возвращает hex-строку SHA-256 от passwd + salt
// эти данные хранятся в БД для авторизации пользователей
std::string hashPasswd(const std::string& passwd, const std::string& salt);
