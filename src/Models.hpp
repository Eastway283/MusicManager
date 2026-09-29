#pragma once

// Данный заголовочный файл содержит определение простейших структур
// для взаимодействия с БД, повторяющих логику и структуру таблиц.

#include <string>

namespace models {

    struct Role {
        int id = 0;
        std::string name;
    };

    struct User {
        int id = 0;
        std::string login;
        std::string passwd_hash;
        std::string salt;
        int role_id = 0;
    };

    struct Band {
        int id = 0;
        std::string name;
        std::string genre;
        std::string description;
        int manager_id = 0;
    };

    struct Musician {
        int id = 0;
        std::string name;
        std::string phone;
        std::string email;
        std::string instrument;
        int band_id = 0;
    };

    struct Venue {
        int id = 0;
        std::string name;
        std::string address;
        int capacity = 0;
        std::string contact;
    };

    struct Concert {
        int id = 0;
        int band_id = 0;
        int venue_id = 0;
        std::string date;
        double fee = 0.0;
        double expenses = 0.0;
        std::string status = "planned";
    };

    struct Contract {
        int id = 0;
        int concert_id = 0;
        std::string number;
        std::string date;
        double amount = 0.0;
        std::string status = "draft";
    };

} // namespace models
