#pragma once

#include "Models.hpp"

// Данный заголовочный файл содержит объявления сервисов-справочников
// взаимодействует с БД в контексте групп, музыкантов и площадок

#include <optional>
#include <string>
#include <vector>

class Database;

// Сервис для работы с таблицей bands: CRUD и выборки
class BandService {
    public:
        explicit BandService(Database& db);

    // Создает запись для группы в БД. Заполняет структуру и возвращает id
    // вновь созданной группы в базе
    // Бросает DbError в случае ошибки базы данных
    int createBand(const std::string& name, const std::string& genre,
                   const std::string& description, int manager_id);

    // Поиск группы в БД по id. возвращает найденную группу
    // nullopt - если группа не найдена в базе данных
    std::optional<models::Band> getById(int id) const;

    // Получение списка всех групп, находящихся в базе данных
    // пустой вектор - если БД пуста
    std::vector<models::Band> getAll() const;

    // Обновление полей группы в БД по id переданной группы
    // Поиск группы осуществляется другими методами
    // При невалидном id ничего не делает
    void update(const models::Band& band);

    // Удаление группы из БД по id
    // При невалидном id - ничего не делает
    // Если на запись ссылаются внешние ключи, то бросает DbError
    void remove(int id);

    // Поиск всех групп, которые имеют одного менеджера
    // Пустой вектор - если у менеджера нету групп
    // ИЛИ manager_id отсутствует в БД
    std::vector<models::Band> findByManager(int manager_id) const;

    // Поиск группы в БД по заданному шаблону
    // Возвращает список групп соответствующих шаблону
    // Пустой вектор - если нет ни одной подходящей группы
    std::vector<models::Band> searchByName(const std::string& part) const;

    private:
    Database& db_;
};


// Сервис для работы с таблицей musicians: CRUD и выборки
class MusicianService {
    public:
        explicit MusicianService(Database& db);

    // Создает запись для музыканта в БД. Заполняет структуру и возвращает id
    // вновь созданного музыканта в базе
    // Бросает DbError в случае ошибки базы данных
    int createMusician(const std::string& name, const std::string& phone, const std::string& email,
                       const std::string& instrument, int band_id);

    // Поиск музыканта в БД по id. возвращает найденного музыканта
    // nullopt - если музыкант не найден в базе данных
    std::optional<models::Musician> getById(int id) const;

    // Получение списка всех музыкантов, находящихся в базе данных
    // пустой вектор - если БД пуста
    std::vector<models::Musician> getAll() const;

    // Обновление полей музыканта в БД по id переданного музыканта
    // Поиск музыканта осуществляется другими методами
    // При невалидном id ничего не делает
    void update(const models::Musician& musician);

    // Удаление музыканта из БД по id
    // При невалидном id - ничего не делает
    // Если на запись ссылаются внешние ключи, то бросает DbError
    void remove(int id);

    // Поиск всех музыкантов, которые состоят в одной группе
    // Пустой вектор - если музыкантов в группе нет
    // ИЛИ band_id отсутствует в БД
    std::vector<models::Musician> findByBand(int band_id) const;

    // Поиск музыканта в БД по заданному шаблону
    // Возвращает список музыкантов соответствующих шаблону
    // Пустой вектор - если нет ни одного подходящего музыканта
    std::vector<models::Musician> searchByName(const std::string& part) const;

    private:
    Database& db_;
};


// Сервис для работы с таблицей venues: CRUD и выборки
class VenueService {
    public:
        explicit VenueService(Database& db);

    // Создает запись для площадки в БД. Заполняет структуру и возвращает id
    // вновь созданной площадки в базе
    // Бросает DbError в случае ошибки базы данных
    int createVenue(const std::string& name, const std::string& address,
                    int capacity, const std::string& contact);

    // Поиск площадки в БД по id. возвращает найденную площадку
    // nullopt - если площадка не найдена в базе данных
    std::optional<models::Venue> getById(int id) const;

    // Получение списка всех площадок, находящихся в базе данных
    // пустой вектор - если БД пуста
    std::vector<models::Venue> getAll() const;

    // Обновление полей площадки в БД по id переданной площадки
    // Поиск площадки осуществляется другими методами
    // При невалидном id ничего не делает
    void update(const models::Venue& venue);

    // Удаление площадки из БД по id
    // При невалидном id - ничего не делает
    // Если на запись ссылаются внешние ключи, то бросает DbError
    void remove(int id);

    // Поиск площадки в БД по заданному шаблону
    // Возвращает список площадок соответствующих шаблону
    // Пустой вектор - если нет ни одной подходящей площадки
    std::vector<models::Venue> searchByName(const std::string& part) const;

    private:
    Database& db_;
};
