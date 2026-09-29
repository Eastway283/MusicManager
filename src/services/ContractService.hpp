#pragma once

#include "Models.hpp"

// Данный заголовочный файл содержит объявление сервиса договоров
// взаимодействует с БД в контексте договоров

#include <optional>
#include <string>
#include <vector>

class Database;

// Сервис для работы с таблицей contracts: CRUD и выборки
class ContractService {
    public:
        explicit ContractService(Database& db);

    // Создает запись договора в БД. Возвращает id вновь созданного договора
    // concert_id должен существовать в БД, иначе бросает DbError
    // Бросает DbError в случае ошибки базы данных
    int createContract(int concert_id, const std::string& number,
                       const std::string& date, double amount);

    // Поиск договора в БД по id. Возвращает найденный договор
    // nullopt - если договор не найден в базе данных
    std::optional<models::Contract> getById(int id) const;

    // Получение списка всех договоров из БД
    // упорядоченных по дате (от новых к старым)
    // пустой вектор - если БД пуста
    std::vector<models::Contract> getAll() const;

    // Обновление полей договора в БД по id переданного договора
    // Поиск договора осуществляется другими методами
    // При невалидном id ничего не делает
    void update(const models::Contract& contract);

    // Удаление договора из БД по id
    // При невалидном id - ничего не делает
    void remove(int id);

    // Поиск договора по id концерта
    // nullopt - если у концерта нет договора
    std::optional<models::Contract> findByConcert(int concert_id) const;

    // Поиск договоров по статусу
    // Статусы: "draft", "signed", "paid", "cancelled"
    // Пустой вектор - если договоров с таким статусом нет
    std::vector<models::Contract> findByStatus(const std::string& status) const;

    // Поиск договоров за период [from; to] включительно
    // Даты в формате "YYYY-MM-DD"
    // Пустой вектор - если в периоде нет договоров
    std::vector<models::Contract> findByDateRange(const std::string& from,
                                                  const std::string& to) const;

    // Изменение статуса договора по id
    // Допустимые статусы: "draft", "signed", "paid", "cancelled"
    // При невалидном id ничего не делает
    void changeStatus(int contract_id, const std::string& new_status);

    private:
    Database& db_;
};
