#pragma once

#include "Models.hpp"
#include "services/AuthService.hpp"
#include "services/CatalogService.hpp"
#include "services/ConcertService.hpp"
#include "services/ContractService.hpp"

// Данный заголовочный файл содержит объявление класса App -
// центрального объекта приложения, который хранит сервисы,
// состояние сессии и владеет окнами интерфейса

class Database;
class LoginWindow;
class MainWindow;

// Класс приложения - хранит все сервисы, состояние сессии и окна.
// Окна получают ссылку на App и через неё обращаются к сервисам
// и навигации между окнами.
class App {
    public:
        explicit App(Database& db);

        // Запускает главный цикл FLTK.
        // Блокирует поток до завершения работы приложения.
        // Перед запуском выполняет seed: если в БД нет пользователей,
        // создаёт администратора admin/admin с ролью admin (id = 1).
        void run();

        // Геттеры для окон (доступ к сервисам и состоянию сессии)
        Database& db()                { return db_; }
        AuthService& auth()           { return auth_; }
        BandService& bands()          { return bands_; }
        MusicianService& musicians()  { return musicians_; }
        VenueService& venues()        { return venues_; }
        ConcertService& concerts()    { return concerts_; }
        ContractService& contracts()  { return contracts_; }

        // Возвращает данные текущего вошедшего пользователя.
        // Невалиден, если isLoggedIn() == false.
        models::User& currentUser()   { return current_user_; }

        // Флаг: true после успешного входа, false после выхода.
        bool isLoggedIn() const       { return logged_in_; }

        // Сохраняет данные вошедшего пользователя, ставит флаг logged_in_
        // и переключает окна: скрывает LoginWindow, показывает MainWindow.
        // Вызывается из LoginWindow при успешной аутентификации.
        void login(const models::User& u);

        // Сбрасывает сессию: очищает current_user_, снимает флаг logged_in_,
        // переключает окна: скрывает MainWindow, показывает LoginWindow.
        // Вызывается из MainWindow при выходе из системы.
        void logout();

    private:
        Database&       db_;
        AuthService     auth_;
        BandService     bands_;
        MusicianService musicians_;
        VenueService    venues_;
        ConcertService  concerts_;
        ContractService contracts_;

        // Окна приложения (владеет App, окна хранят ссылку на App)
        // Указатели: FLTK-окна создаются через new и живут до конца программы
        LoginWindow*    login_window_ = nullptr;
        MainWindow*     main_window_  = nullptr;

        // Состояние сессии
        models::User    current_user_;          // вошедший пользователь
        bool            logged_in_    = false;  // флаг авторизации
};
