#pragma once

#include <string>

// Функции перевода статусов между внутренним (английским)
// и отображаемым (русским) представлениями.
// В БД всегда хранятся английские значения — они стабильны и не зависят
// от локализации интерфейса. В UI показываем русские названия.

// Концерты: planned / done / cancelled
std::string concertStatusToRu(const std::string& status);
std::string concertStatusFromRu(const std::string& ru_status);

// Договоры: draft / signed / paid / cancelled
std::string contractStatusToRu(const std::string& status);
std::string contractStatusFromRu(const std::string& ru_status);
