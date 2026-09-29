#include <cstdint>
#include "util/Hash.hpp"
#include <string>
#include <random>
#include <picosha2.h>

// локальная функция для преобразования числа 0...15 в hex вид
// необходима для генерации соли
static char hexDigit(uint8_t nibble) {
    return nibble < 10 ? ('0' + nibble) : ('a' + nibble - 10);
}

// генерирует соль: 16 случайных байт, закодированных в hex.
// возвращает строку из 32 символов
std::string generateSalt() {
    std::string salt;
    salt.reserve(32);
    std::random_device rd;

    for (int i = 0; i < 16; i++) {
        uint8_t b = static_cast<uint8_t>(rd());
        salt.push_back(hexDigit(b >> 4));
        salt.push_back(hexDigit(b & 0x0F));
    }
    return salt;
}

// считает SHA-256 от (passwd + salt), возвращает hex-строку из 64 символов
std::string hashPasswd(const std::string& passwd, const std::string& salt) {
    return picosha2::hash256_hex_string(passwd + salt);
}
