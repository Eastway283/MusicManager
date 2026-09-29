#include "services/ContractService.hpp"
#include "db/Database.hpp"
#include "db/Statement.hpp"
#include "Models.hpp"
#include "sqlite3.h"

#include <string>
#include <optional>
#include <vector>

// статическая функция для заполнения записи договора
// используется в getById, getAll, findByConcert, findByStatus, findByDateRange
// применяется для избежания дублирования кода
static models::Contract rowToContract(Statement& s) {
    models::Contract c;
    c.id         = s.columnInt(0);
    c.concert_id = s.columnInt(1);
    c.number     = s.columnText(2);
    c.date       = s.columnText(3);
    c.amount     = s.columnDouble(4);
    c.status     = s.columnText(5);
    return c;
}

ContractService::ContractService(Database& db) : db_(db) {}

int ContractService::createContract(int concert_id, const std::string& number,
                                    const std::string& date, double amount) {
    // Формирование запроса к БД contracts для создания договора
    // status не передаётся - подставляется DEFAULT 'draft' из схемы
    Statement s(db_, "INSERT INTO contracts (concert_id, number, date, amount) "
                     "VALUES (?, ?, ?, ?)");
    // добавление значений к запросу
    s.bind(1, concert_id)
     .bind(2, number)
     .bind(3, date)
     .bind(4, amount);

    // непосредственная вставка в БД
    s.step();

    // возврат id только что созданной записи
    return static_cast<int>(sqlite3_last_insert_rowid(db_.handle()));
}

std::optional<models::Contract> ContractService::getById(int id) const {
    // Формирование запроса к БД для поиска договора по id
    Statement s(db_, "SELECT id, concert_id, number, date, amount, status "
                     "FROM contracts WHERE id = ?");
    s.bind(1, id); // добавление id к запросу
    if (!s.step()) // такого договора нет - возврат nullopt
        return std::nullopt;
    // Возврат готовой записи (заполнение внутри rowToContract)
    return rowToContract(s);
}

std::vector<models::Contract> ContractService::getAll() const {
    std::vector<models::Contract> res;
    // Формирование запроса для получения ВСЕХ записей из БД contracts
    // упорядоченных по дате (от новых к старым)
    Statement s(db_, "SELECT id, concert_id, number, date, amount, status "
                     "FROM contracts ORDER BY date DESC");
    // Перебор каждой записи в цикле и добавление её в список
    while (s.step())
        res.push_back(rowToContract(s));
    // Возврат списка ВСЕХ договоров
    return res;
}

void ContractService::update(const models::Contract& contract) {
    // Формирование запроса к БД для изменения записи по id договора
    Statement s(db_, "UPDATE contracts SET concert_id = ?, number = ?, date = ?, amount = ?, "
                     "status = ? WHERE id = ?");
    // Добавление переменных к запросу в заданном порядке
    s.bind(1, contract.concert_id)
     .bind(2, contract.number)
     .bind(3, contract.date)
     .bind(4, contract.amount)
     .bind(5, contract.status)
     .bind(6, contract.id);

    // При отсутствии договора в БД - ничего не произойдет
    s.step();
}

void ContractService::remove(int id) {
    // Формирование запроса к БД для удаления договора по id
    Statement s(db_, "DELETE FROM contracts WHERE id = ?");
    s.bind(1, id);
    // При отсутствии договора с заданным id - ничего не произойдет
    s.step();
}

std::optional<models::Contract> ContractService::findByConcert(int concert_id) const {
    // Формирование запроса для поиска договора по id концерта
    Statement s(db_, "SELECT id, concert_id, number, date, amount, status "
                     "FROM contracts WHERE concert_id = ?");
    s.bind(1, concert_id);
    if (!s.step()) // договора для этого концерта нет - возврат nullopt
        return std::nullopt;
    // Возврат готовой записи
    return rowToContract(s);
}

std::vector<models::Contract> ContractService::findByStatus(const std::string& status) const {
    std::vector<models::Contract> res;
    // Формирование запроса для поиска всех договоров с заданным статусом
    // Статусы: draft / signed / paid / cancelled
    Statement s(db_, "SELECT id, concert_id, number, date, amount, status "
                     "FROM contracts WHERE status = ? ORDER BY date DESC");
    s.bind(1, status);
    // Заполнение списка для возврата в цикле
    while (s.step())
        res.push_back(rowToContract(s));
    return res;
}

std::vector<models::Contract> ContractService::findByDateRange(const std::string& from,
                                                               const std::string& to) const {
    std::vector<models::Contract> res;
    // Формирование запроса для поиска договоров в диапазоне [from; to]
    // BETWEEN включает оба конца, даты сравниваются лексикографически
    Statement s(db_, "SELECT id, concert_id, number, date, amount, status "
                     "FROM contracts WHERE date BETWEEN ? AND ? ORDER BY date");
    s.bind(1, from)
     .bind(2, to);
    // Заполнение списка для возврата в цикле
    while (s.step())
        res.push_back(rowToContract(s));
    return res;
}

void ContractService::changeStatus(int contract_id, const std::string& new_status) {
    // Формирование запроса к БД для изменения статуса договора
    // Допустимые значения статуса обеспечиваются со стороны пользователя 
    Statement s(db_, "UPDATE contracts SET status = ? WHERE id = ?");
    s.bind(1, new_status)
     .bind(2, contract_id);

    // При отсутствии договора с заданным id - ничего не произойдет
    s.step();
}
