#include "ui/Windows.hpp"
#include "ui/App.hpp"
#include "ui/Panels.hpp"

#include <FL/Enumerations.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>

// --- LoginWindow ---

// Конструктор окна входа: создаёт поля логина/пароля и кнопки.
// Кнопка "Войти" привязана к onLoginClicked, кнопка "Выход" завершает приложение.
LoginWindow::LoginWindow(App& app) : Fl_Window(340, 180, "Вход в систему"), app_(app) {
    // Поле "Логин"
    input_login_ = new Fl_Input(120, 30, 180, 25, "Логин");
    input_login_->align(FL_ALIGN_LEFT);

    // Поле "Пароль" - с отключением эха
    input_passwd_ = new Fl_Input(120, 65, 180, 25, "Пароль");
    input_passwd_->align(FL_ALIGN_LEFT);
    input_passwd_->type(FL_SECRET_INPUT);

    // Кнопка "Войти"
    button_login_ = new Fl_Button(120, 110, 100, 30, "Войти");
    button_login_->callback(onLoginClicked, this);

    // Кнопка "Выход" - завершает приложение
    auto button_quit = new Fl_Button(230, 110, 70, 30, "Выход");
    button_quit->callback([] (Fl_Widget*, void*) { exit(0); });

    // завершаем добавление виджетов
    end();
}

// Статический callback кнопки "Войти".
// Получает this через data и вызывает attemptLogin().
void LoginWindow::onLoginClicked(Fl_Widget*, void *data) {
    auto* self = static_cast<LoginWindow*>(data);
    self->attemptLogin();
}

// Читает поля, вызывает AuthService::login().
// При успехе - App::login() (переключает окна).
// При неудаче - fl_alert с сообщением об ошибке.
void LoginWindow::attemptLogin() {
    std::string login  = input_login_->value() ? input_login_->value() : "";
    std::string passwd = input_passwd_->value() ? input_passwd_->value() : "";

    auto user = app_.auth().login(login, passwd);
    if (user)
        app_.login(*user);
    else
        fl_alert("Неверный логин или пароль");
}

// Очищает поля логина и пароля.
// Вызывается из App::logout() при возврате на экран входа.
void LoginWindow::clearFields() {
    input_login_ ->value("");
    input_passwd_->value("");
}

// --- MainWindow ---

// Вспомогательная структура для передачи в callback пункта меню.
// FLTK позволяет передать только один void* через data, а нам нужно
// два значения: тип панели и указатель на MainWindow.
struct PanelMenuItem {
    MainWindow* self;
    PanelType type;
};

// Конструктор главного окна: создаёт верхнее меню и рабочую область.
// Пункты меню связаны с onPanelSelected через массив PanelMenuItem.
MainWindow::MainWindow(App& app) : Fl_Window(800, 600, "Music Manager"), app_(app) {

    // Статический массив: живёт после выхода из конструктора,
    // потому что FLTK сохраняет указатели на его элементы в data пунктов меню.
    static PanelMenuItem items[] = {
    { this, PanelType::Bands     },
    { this, PanelType::Musicians },
    { this, PanelType::Venues    },
    { this, PanelType::Concerts  },
    { this, PanelType::Contracts },
    { this, PanelType::Reports   },
    { this, PanelType::Users     },
    };

    // Создаение меню
    menu_ = new Fl_Menu_Bar(0, 0, 800, 25);
    menu_->add("Файл/Выход", 0, onLogout, this);

    menu_->add("Справочники/Группы",       0, onPanelSelected, &items[0]);
    menu_->add("Справочники/Музыканты",    0, onPanelSelected, &items[1]);
    menu_->add("Справочники/Площадки",     0, onPanelSelected, &items[2]);
    menu_->add("Концерты",     0, onPanelSelected, &items[3]);
    menu_->add("Контракты",    0, onPanelSelected, &items[4]);
    menu_->add("Отчеты",       0, onPanelSelected, &items[5]);
    menu_->add("Пользователи", 0, onPanelSelected, &items[6]);

    // Создание рабочей области.
    workspace_ = new Fl_Group(0, 25, 800, 575);
    workspace_->box(FL_FLAT_BOX);
    workspace_->color(FL_BACKGROUND_COLOR);
    workspace_->end();

    end();

    showPanel(PanelType::None);
}

// Делегирует логику выхода в App::logout().
void MainWindow::doLogout() { app_.logout(); }

// Переключает рабочую область на панель указанного типа.
// Удаляет предыдущий виджет из workspace_, создаёт новый,
// добавляет его в рабочую область и перерисовывает окно.
void MainWindow::showPanel(PanelType type) {
    // Удаление старого
    if (current_widget_) {
        workspace_->remove(current_widget_);
        delete current_widget_;
        current_widget_ = nullptr;
    }
    // Создание нового
    switch (type) {
        case PanelType::Bands:
            current_widget_ = new BandPanel(0, 25, 800, 575, app_);
            break;
    // остальные пока заглушки:
        case PanelType::Musicians:
            current_widget_ = new MusicianPanel(0, 25, 800, 575, app_);
            break;
        case PanelType::Venues:
            current_widget_ = new VenuePanel(0, 25, 800, 575, app_);
            break;
        case PanelType::Concerts:
            current_widget_ = new ConcertPanel(0, 25, 800, 575, app_);
        break;
        case PanelType::Contracts:
            current_widget_ = new ContractPanel(0, 25, 800, 575, app_);
            break;
        case PanelType::Reports:
            current_widget_ = new ReportPanel(0, 25, 800, 575, app_);
            break;
        case PanelType::Users:
            current_widget_ = new UserPanel(0, 25, 800, 575, app_);
            break;
        case PanelType::None:
            current_widget_ = new Fl_Box(0, 25, 800, 475, "Система управления базами данных "
                                                          "для менеджера музыкальных групп\n\n"
                                                          "Выберите раздел в меню сверху\n\n"
                                                          "Справочники - группы, музыканты, площадки\n"
                                                          "Концерты - планирование выступлений\n"
                                                          "Договоры - учет заключенных сделок\n"
                                                          "Отчёты - сводные данные\n"
                                                          "Пользователи - управление учетными записями "
                                                          "(только админ)");
            current_widget_->labelsize(16);
            current_widget_->align(FL_ALIGN_CENTER);
            current_widget_->labelfont(FL_BOLD);
            break;
        default:
            current_widget_ = new Fl_Box(0, 25, 800, 575, "В разработке");
            current_widget_->labelsize(25);
        break;
    }
    // Добавляем в рабочую область и перерисовываем окно целиком.
    workspace_->add(current_widget_);
    redraw();

    current_panel_ = type;
}

void MainWindow::reset() {
    // Сбросить рабочую область на заглушку
    showPanel(PanelType::None);

    // Снять выделение в меню
    // Без этого ранее выбранный пункт может остаться подсвеченным.
    menu_->value(nullptr);
    redraw();
}

// Статический callback пункта меню "Файл -> Выход".
// Получает this через data и вызывает doLogout().
void MainWindow::onLogout(Fl_Widget*, void* data) {
    auto* self = static_cast<MainWindow*>(data);
    self->doLogout();
}

// Статический callback пунктов меню разделов.
// Через data получает PanelMenuItem и вызывает showPanel().
void MainWindow::onPanelSelected(Fl_Widget*, void* data) {
    auto* item = static_cast<PanelMenuItem*>(data);

    // Проверка прав: раздел "Пользователи" только для администратора
    if (item->type == PanelType::Users && item->self->app_.currentUser().role_id != 1) {
        fl_alert("Доступ только для администратора");
        return;
    }

    item->self->showPanel(item->type);
}
