#include "db/Database.hpp"
#include "db/Statement.hpp"
#include "services/ConcertService.hpp"
#include "util/Date.hpp"
#include "Models.hpp"
#include "sqlite3.h"

#include <string>
#include <vector>
#include <optional>

// статическая функция для заполнения записи концерта
// используется в getById, getAll, findByBand, findByVenue,
// findByDateRange, findByStatus, findUpcoming
// применяется для избежания дублирования кода
static models::Concert rowToConcert(Statement& s) {
    models::Concert c;
    c.id       = s.columnInt(0);
    c.band_id  = s.columnInt(1);
    c.venue_id = s.columnInt(2);
    c.date     = s.columnText(3);
    c.fee      = s.columnDouble(4);
    c.expenses = s.columnDouble(5);
    c.status   = s.columnText(6);
    return c;
}

ConcertService::ConcertService(Database& db) : db_(db) {}

int ConcertService::createConcert(int band_id, int venue_id,
                                  const std::string& date,
                                  double fee, double expenses) {
    // Формирование запроса к БД concerts для создания концерта
    // band_id и venue_id должны существовать в БД,
    // иначе SQLite бросит DbError (FOREIGN KEY constraint failed)
    // status не передаётся - подставляется DEFAULT 'planned' из схемы
    Statement s(db_, "INSERT INTO concerts (band_id, venue_id, date, fee, expenses) "
                     "VALUES (?, ?, ?, ?, ?)");
    // добавление значений к запросу
    s.bind(1, band_id)
     .bind(2, venue_id)
     .bind(3, date)
     .bind(4, fee)
     .bind(5, expenses);

    // непосредственная вставка в БД
    s.step();

    // возврат id только что созданной записи
    return static_cast<int>(sqlite3_last_insert_rowid(db_.handle()));
}

std::optional<models::Concert> ConcertService::getById(int id) const {
    // Формирование запроса к БД для поиска концерта по id
    Statement s(db_, "SELECT id, band_id, venue_id, date, fee, expenses, status "
                     "FROM concerts WHERE id = ?");
    s.bind(1, id); // добавление id к запросу
    if (!s.step()) // такого концерта нет - возврат nullopt
        return std::nullopt;
    // Возврат готовой записи (заполнение внутри rowToConcert)
    return rowToConcert(s);
}

std::vector<models::Concert> ConcertService::getAll() const {
    std::vector<models::Concert> res;
    // Формирование запроса для получения ВСЕХ записей из БД concerts
    // упорядоченных по дате (от новых к старым)
    Statement s(db_, "SELECT id, band_id, venue_id, date, fee, expenses, status "
                     "FROM concerts ORDER BY date DESC");
    // Перебор каждой записи в цикле и добавление её в список
    while (s.step())
        res.push_back(rowToConcert(s));
    // Возврат списка ВСЕХ концертов
    return res;
}

void ConcertService::update(const models::Concert& concert) {
    // Формирование запроса к БД для изменения записи по id концерта
    Statement s(db_, "UPDATE concerts SET band_id = ?, venue_id = ?, date = ?, fee = ?, "
                     "expenses = ?, status = ? WHERE id = ?");
    // Добавление переменных к запросу в заданном порядке
    s.bind(1, concert.band_id)
     .bind(2, concert.venue_id)
     .bind(3, concert.date)
     .bind(4, concert.fee)
     .bind(5, concert.expenses)
     .bind(6, concert.status)
     .bind(7, concert.id);

    // При отсутствии концерта в БД - ничего не произойдет
    s.step();
}

void ConcertService::remove(int id) {
    // Формирование запроса к БД для удаления концерта по id
    Statement s(db_, "DELETE FROM concerts WHERE id = ?");
    s.bind(1, id);
    // При отсутствии концерта с заданным id - ничего не произойдет
    // Если на концерт ссылается договор - бросает DbError
    s.step();
}

std::vector<models::Concert> ConcertService::findByBand(int band_id) const {
    std::vector<models::Concert> res;
    // Формирование запроса для получения всех концертов заданной группы
    Statement s(db_, "SELECT id, band_id, venue_id, date, fee, expenses, status "
                     "FROM concerts WHERE band_id = ? ORDER BY date");
    s.bind(1, band_id);
    // Заполнение списка для возврата в цикле
    while (s.step())
        res.push_back(rowToConcert(s));
    return res;
}

std::vector<models::Concert> ConcertService::findByVenue(int venue_id) const {
    std::vector<models::Concert> res;
    // Формирование запроса для получения всех концертов на заданной площадке
    Statement s(db_, "SELECT id, band_id, venue_id, date, fee, expenses, status "
                     "FROM concerts WHERE venue_id = ? ORDER BY date");
    s.bind(1, venue_id);
    // Заполнение списка для возврата в цикле
    while (s.step())
        res.push_back(rowToConcert(s));
    return res;
}

std::vector<models::Concert> ConcertService::findByDateRange(const std::string& from,
                                                             const std::string& to) const {
    std::vector<models::Concert> res;
    // Формирование запроса для поиска концертов в диапазоне [from; to]
    // BETWEEN включает оба конца, даты сравниваются лексикографически
    Statement s(db_, "SELECT id, band_id, venue_id, date, fee, expenses, status "
                     "FROM concerts WHERE date BETWEEN ? AND ? ORDER BY date");
    s.bind(1, from)
     .bind(2, to);
    // Заполнение списка для возврата в цикле
    while (s.step())
        res.push_back(rowToConcert(s));
    return res;
}

std::vector<models::Concert> ConcertService::findUpcoming() const {
    std::vector<models::Concert> res;
    // Формирование запроса для поиска предстоящих концертов (date >= сегодня)
    // Сегодняшняя дата вычисляется через today() и передаётся как параметр
    Statement s(db_, "SELECT id, band_id, venue_id, date, fee, expenses, status "
                     "FROM concerts WHERE date >= ? ORDER BY date");
    s.bind(1, today());
    // Заполнение списка для возврата в цикле
    while (s.step())
        res.push_back(rowToConcert(s));
    return res;
}

std::vector<models::Concert> ConcertService::findByStatus(const std::string& status) const {
    std::vector<models::Concert> res;
    // Формирование запроса для поиска концертов с заданным статусом
    // Статусы: planned / done / cancelled
    Statement s(db_, "SELECT id, band_id, venue_id, date, fee, expenses, status "
                     "FROM concerts WHERE status = ? ORDER BY date");
    s.bind(1, status);
    // Заполнение списка для возврата в цикле
    while (s.step())
        res.push_back(rowToConcert(s));
    return res;
}
