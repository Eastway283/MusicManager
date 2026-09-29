#pragma once

#include "Models.hpp"

// Данный заголовочный файл содержит объявление сервиса концертов
// взаимодействует с БД в контексте концертов

#include <optional>
#include <string>
#include <vector>

class Database;

// Сервис для работы с таблицей concerts: CRUD и выборки
// Концерт связан с группой (band_id) и площадкой (venue_id)
class ConcertService {
    public:
        explicit ConcertService(Database& db);

    // Создает запись концерта в БД. Возвращает id вновь созданного концерта
    // band_id и venue_id должны существовать в БД, иначе бросает DbError
    // Бросает DbError в случае ошибки базы данных
    int createConcert(int band_id, int venue_id, const std::string& date,
                      double fee, double expenses);

    // Поиск концерта в БД по id. Возвращает найденный концерт
    // nullopt - если концерт не найден в базе данных
    std::optional<models::Concert> getById(int id) const;

    // Получение списка всех концертов из БД
    // упорядоченных по дате (от новых к старым)
    // пустой вектор - если БД пуста
    std::vector<models::Concert> getAll() const;

    // Обновление полей концерта в БД по id переданного концерта
    // Поиск концерта осуществляется другими методами
    // При невалидном id ничего не делает
    void update(const models::Concert& concert);

    // Удаление концерта из БД по id
    // При невалидном id - ничего не делает
    // Если на запись ссылаются внешние ключи (например, договор),
    // то бросает DbError
    void remove(int id);

    // Поиск всех концертов заданной группы
    // Пустой вектор - если у группы нет концертов
    // ИЛИ band_id отсутствует в БД
    std::vector<models::Concert> findByBand(int band_id) const;

    // Поиск всех концертов на заданной площадке
    // Пустой вектор - если на площадке нет концертов
    // ИЛИ venue_id отсутствует в БД
    std::vector<models::Concert> findByVenue(int venue_id) const;

    // Поиск концертов за период [from; to] включительно
    // Даты в формате "YYYY-MM-DD" (например, "2026-10-01")
    // Пустой вектор - если в периоде нет концертов
    std::vector<models::Concert> findByDateRange(const std::string& from, const std::string& to) const;

    // Поиск концертов по статусу
    // Статусы: "planned", "done", "cancelled"
    // Пустой вектор - если концертов с таким статусом нет
    std::vector<models::Concert> findByStatus(const std::string& status) const;

    // Поиск предстоящих концертов (дата >= сегодня)
    // Сегодняшняя дата определяется на стороне C++ и передаётся в SQL
    // Пустой вектор - если предстоящих концертов нет
    std::vector<models::Concert> findUpcoming() const;

    private:
    Database& db_;
};
