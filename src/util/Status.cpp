#include "util/Status.hpp"

// Преобразование статуса концерта: английский -> русский.
// Если значение неизвестно — возвращаем его как есть.
std::string concertStatusToRu(const std::string& status) {
    if (status == "planned")
        return "Запланирован";
    if (status == "done")
        return "Проведён";
    if (status == "cancelled")
        return "Отменён";
    return status;
}

// Преобразование статуса концерта: русский -> английский.
// Если значение неизвестно — возвращаем как есть
// (это позволит отловить опечатки при отладке).
std::string concertStatusFromRu(const std::string& ru_status) {
    if (ru_status == "Запланирован")
        return "planned";
    if (ru_status == "Проведён")
        return "done";
    if (ru_status == "Отменён")
        return "cancelled";
    return ru_status;
}

// Преобразование статуса договора: английский -> русский.
std::string contractStatusToRu(const std::string& status) {
    if (status == "draft")
        return "Черновик";
    if (status == "signed")
        return "Подписан";
    if (status == "paid")
        return "Оплачен";
    if (status == "cancelled")
        return "Расторгнут";
    return status;
}

// Преобразование статуса договора: русский -> английский.
std::string contractStatusFromRu(const std::string& ru_status) {
    if (ru_status == "Черновик")
        return "draft";
    if (ru_status == "Подписан")
        return "signed";
    if (ru_status == "Оплачен")
        return "paid";
    if (ru_status == "Расторгнут")
        return "cancelled";
    return ru_status;
}
