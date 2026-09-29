#include "ui/App.hpp"
#include "ui/Windows.hpp"
#include "services/AuthService.hpp"
#include "db/Database.hpp"

#include <FL/Fl.H>

App::App(Database& db) : db_(db), auth_(db), bands_(db), musicians_(db),
                         venues_(db), concerts_(db), contracts_(db) {}

void App::login(const models::User& u) {
    current_user_ = u;
    logged_in_    = true;
    login_window_->hide();
    main_window_ ->show();
}

void App::logout() {
    current_user_ = models::User{};
    logged_in_    = false;
    login_window_->clearFields();
    main_window_ ->reset();
    main_window_ ->hide();
    login_window_->show();
}

void App::run() {
    if (!auth_.hasAnyUser())
        auth_.createUser("admin", "admin", 1);

    login_window_ = new LoginWindow(*this);
    main_window_  = new MainWindow(*this);

    login_window_->show();

    Fl::run();
}
