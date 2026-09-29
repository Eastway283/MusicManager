#include "db/Database.hpp"
#include "db/Statement.hpp"
#include "services/CatalogService.hpp"
#include "Models.hpp"
#include "sqlite3.h"

#include <string>
#include <vector>
#include <optional>

// Преобразует текущую строку Statement в структуру Band
// данный фрагмент кода используется в getById, getAll, findByManager, searchByName
// используется для избежания дублирования кода
static models::Band rowToBand(Statement& s) {
    models::Band b;
    b.id          = s.columnInt(0);
    b.name        = s.columnText(1);
    b.genre       = s.columnText(2);
    b.description = s.columnText(3);
    b.manager_id  = s.columnInt(4);
    return b;
}

BandService::BandService(Database& db) : db_(db) {}

int BandService::createBand(const std::string& name, const std::string& genre,
                            const std::string& description, int manager_id) {
    // Формирование запроса к БД bands, для создания группы с заданными полями
    Statement s(db_, "INSERT INTO bands (name, genre, description, manager_id) "
                     "VALUES (?, ?, ?, ?)");
    // добавление значений к запросу
    s.bind(1, name)
     .bind(2, genre)
     .bind(3, description)
     .bind(4, manager_id);

    // непосредственная вставка в БД
    s.step();

    // возврат id только что созданной записи в БД
    return static_cast<int>(sqlite3_last_insert_rowid(db_.handle()));
}

std::optional<models::Band> BandService::getById(int id) const {
    // Формирование запроса к БД для поиска группы по id
    Statement s(db_, "SELECT id, name, genre, description, manager_id "
                     "FROM bands WHERE id = ?");
    s.bind(1, id); // добавление id к запросу
    if (!s.step()) // такой группы нет - возврат nullopt
        return std::nullopt;
    // Объявление и инициализация структуры
    // данными записи из БД (внутри rowToBand)
    // Возврат готовой записи
    return rowToBand(s);
}

std::vector<models::Band> BandService::getAll() const {
    std::vector<models::Band> res;
    // Формирование запроса к БД для получения ВСЕХ записей из БД bands
    // упорядоченных по имени
    Statement s(db_, "SELECT id, name, genre, description, manager_id FROM bands ORDER BY name");
    // Перебор каждой записи в цикле и добавление ее в список
    while (s.step())
        res.push_back(rowToBand(s));
    // Возврат списка ВСЕХ групп
    return res;
}

void BandService::update(const models::Band& band) {
    // Формирование запроса к БД для изменения записи 
    // по id группы
    Statement s(db_, "UPDATE bands SET name = ?, genre = ?, description = ?, "
                     "manager_id = ? WHERE id = ?");
    // Добавление переменных к запросу в заданном в запросе порядке
    s.bind(1, band.name)
     .bind(2, band.genre)
     .bind(3, band.description)
     .bind(4, band.manager_id)
     .bind(5, band.id);
    // При отсутствии группы в БД - ничего не произойдет
    s.step();
}

void BandService::remove(int id) {
    // Формирование запроса к БД для удаления группы по id
    Statement s(db_, "DELETE FROM bands WHERE id = ?");
    s.bind(1, id);
    // При отсутствии группы с заданным id - ничего не произойдет
    s.step();
}

std::vector<models::Band> BandService::findByManager(int manager_id) const {
    std::vector<models::Band> res;
    // Формирование запроса к БД для получения всех записей с заданым manager_id
    Statement s(db_, "SELECT id, name, genre, description, manager_id "
                     "FROM bands WHERE manager_id = ? ORDER BY name");
    s.bind(1, manager_id);
    // Заполнение записи группы в цикле
    // и добавление ее в список для возврата
    while (s.step())
        res.push_back(rowToBand(s));
    return res;
}

std::vector<models::Band> BandService::searchByName(const std::string& part) const {
    std::vector<models::Band> res;
    // Формирование запроса к БД для поиска всех записей
    // имена которых соответствуют подстроке part
    Statement s(db_, "SELECT id, name, genre, description, manager_id "
                     "FROM bands WHERE name LIKE ? ORDER BY name");
    // Конкатенация подстроки для формирования шаблона
    s.bind(1, "%" + part + "%");
    // Заполнение списка для возврата в цикле
    while (s.step())
        res.push_back(rowToBand(s));
    return res;
}

// статическая функция для заполнения записи музыканта
// используется в getById, getAll, findByBand, searchByName
// применяется для избежания дублирования кода
static models::Musician rowToMusician(Statement& s) {
    models::Musician m;
    m.id         = s.columnInt(0);
    m.name       = s.columnText(1);
    m.phone      = s.columnText(2);
    m.email      = s.columnText(3);
    m.instrument = s.columnText(4);
    m.band_id    = s.columnInt(5);
    return m;
}

MusicianService::MusicianService(Database& db) : db_(db) {}

int MusicianService::createMusician(const std::string& name, const std::string& phone, const std::string& email,
                                    const std::string& instrument, int band_id) {
    // Формирование запроса к БД musicians для создания музыканта с заданными полями
    Statement s(db_, "INSERT INTO musicians (name, phone, email, instrument, band_id) "
                     "VALUES (?, ?, ?, ?, ?)");
    // добавление значений к запросу
    s.bind(1, name)
     .bind(2, phone)
     .bind(3, email)
     .bind(4, instrument)
     .bind(5, band_id);

    // непосредственная вставка в БД
    s.step();

    // возврат id только что созданной записи
    return static_cast<int>(sqlite3_last_insert_rowid(db_.handle()));
}

std::optional<models::Musician> MusicianService::getById(int id) const {
    // Формирование запроса к БД для поиска музыканта по id
    Statement s(db_, "SELECT id, name, phone, email, instrument, band_id "
                     "FROM musicians WHERE id = ?");
    s.bind(1, id); // добавление id к запросу
    if (!s.step()) // такого музыканта нет - возврат nullopt
        return std::nullopt;
    // Возврат готовой записи (заполнение внутри rowToMusician)
    return rowToMusician(s);
}

std::vector<models::Musician> MusicianService::getAll() const {
    std::vector<models::Musician> res;
    // Формирование запроса для получения ВСЕХ записей из БД musicians
    // упорядоченных по имени
    Statement s(db_, "SELECT id, name, phone, email, instrument, band_id "
                     "FROM musicians ORDER BY name");
    // Перебор каждой записи в цикле и добавление её в список
    while (s.step())
        res.push_back(rowToMusician(s));
    // Возврат списка ВСЕХ музыкантов
    return res;
}

void MusicianService::update(const models::Musician& musician) {
    // Формирование запроса к БД для изменения записи по id музыканта
    Statement s(db_, "UPDATE musicians SET name = ?, phone = ?, email = ?, "
                     "instrument = ?, band_id = ? WHERE id = ?");
    // Добавление переменных к запросу в заданном порядке
    s.bind(1, musician.name)
     .bind(2, musician.phone)
     .bind(3, musician.email)
     .bind(4, musician.instrument)
     .bind(5, musician.band_id)
     .bind(6, musician.id);

    // При отсутствии музыканта в БД - ничего не произойдет
    s.step();
}

void MusicianService::remove(int id) {
    // Формирование запроса к БД для удаления музыканта по id
    Statement s(db_, "DELETE FROM musicians WHERE id = ?");
    s.bind(1, id);
    // При отсутствии музыканта с заданным id - ничего не произойдет
    s.step();
}

std::vector<models::Musician> MusicianService::findByBand(int band_id) const {
    std::vector<models::Musician> res;
    // Формирование запроса для получения всех записей с заданным band_id
    Statement s(db_, "SELECT id, name, phone, email, instrument, band_id "
                     "FROM musicians WHERE band_id = ? ORDER BY name");
    s.bind(1, band_id);
    // Заполнение записи музыканта в цикле и добавление её в список
    while (s.step())
        res.push_back(rowToMusician(s));
    return res;
}

std::vector<models::Musician> MusicianService::searchByName(const std::string& part) const {
    std::vector<models::Musician> res;
    // Формирование запроса для поиска всех записей,
    // имена которых соответствуют подстроке part
    Statement s(db_, "SELECT id, name, phone, email, instrument, band_id "
                     "FROM musicians WHERE name LIKE ? ORDER BY name");
    // Конкатенация подстроки для формирования шаблона
    s.bind(1, "%" + part + "%");
    // Заполнение списка для возврата в цикле
    while (s.step())
        res.push_back(rowToMusician(s));
    return res;
}

// Cтатическая функция для заполнения записи площадки
// используется в getById, getAll, searchByName
// применяется для избежания дублирования кода
static models::Venue rowToVenue(Statement& s) {
    models::Venue v;
    v.id       = s.columnInt(0);
    v.name     = s.columnText(1);
    v.address  = s.columnText(2);
    v.capacity = s.columnInt(3);
    v.contact  = s.columnText(4);
    return v;
}

VenueService::VenueService(Database& db) : db_(db) {}

int VenueService::createVenue(const std::string& name, const std::string& address,
                              int capacity, const std::string& contact) {
    // Формирование запроса к БД venues для создания площадки с заданными полями
    Statement s(db_, "INSERT INTO venues (name, address, capacity, contact) "
                     "VALUES (?, ?, ?, ?)");
    // Добавление значений к запросу
    s.bind(1, name)
     .bind(2, address)
     .bind(3, capacity)
     .bind(4, contact);

    // Непосредственная вставка в БД
    s.step();

    // Возврат id только что созданной записи
    return static_cast<int>(sqlite3_last_insert_rowid(db_.handle()));
}

std::optional<models::Venue> VenueService::getById(int id) const {
    // Формирование запроса к БД для поиска площадки по id
    Statement s(db_, "SELECT id, name, address, capacity, contact "
                     "FROM venues WHERE id = ?");
    s.bind(1, id); // Добавление id к запросу
    if (!s.step()) // Такой площадки нет - возврат nullopt
        return std::nullopt;
    // Возврат готовой записи (заполнение внутри rowToVenue)
    return rowToVenue(s);
}

std::vector<models::Venue> VenueService::getAll() const {
    std::vector<models::Venue> res;
    // Формирование запроса для получения ВСЕХ записей из БД venues
    // упорядоченных по имени
    Statement s(db_, "SELECT id, name, address, capacity, contact "
                     "FROM venues ORDER BY name");
    // Перебор каждой записи в цикле и добавление её в список
    while (s.step())
        res.push_back(rowToVenue(s));
    // Возврат списка ВСЕХ площадок
    return res;
}

void VenueService::update(const models::Venue& venue) {
    // Формирование запроса к БД для изменения записи по id площадки
    Statement s(db_, "UPDATE venues SET name = ?, address = ?, capacity = ?, "
                     "contact = ? WHERE id = ?");
    // Добавление переменных к запросу в заданном порядке
    s.bind(1, venue.name)
     .bind(2, venue.address)
     .bind(3, venue.capacity)
     .bind(4, venue.contact)
     .bind(5, venue.id);

    // При отсутствии площадки в БД - ничего не произойдет
    s.step();
}

void VenueService::remove(int id) {
    // Формирование запроса к БД для удаления площадки по id
    Statement s(db_, "DELETE FROM venues WHERE id = ?");
    s.bind(1, id);
    // При отсутствии площадки с заданным id - ничего не произойдет
    s.step();
}

std::vector<models::Venue> VenueService::searchByName(const std::string& part) const {
    std::vector<models::Venue> res;
    // Формирование запроса для поиска всех записей,
    // имена которых соответствуют подстроке part
    Statement s(db_, "SELECT id, name, address, capacity, contact "
                     "FROM venues WHERE name LIKE ? ORDER BY name");
    // Конкатенация подстроки для формирования шаблона
    s.bind(1, "%" + part + "%");
    // Заполнение списка для возврата в цикле
    while (s.step())
        res.push_back(rowToVenue(s));
    return res;
}
