#pragma once

#include <FL/Fl_Window.H>

// Данный заголовочный файл содержит объявления окон интерфейса:
// окна входа в систему и главного окна приложения.
// Оба окна хранят ссылку на App и через неё обращаются к сервисам
// и навигации между окнами.

class Fl_Input;
class Fl_Button;
class Fl_Menu_Bar;
class Fl_Group;
class Fl_Widget;
class App;

// Окно входа в систему: поля логина и пароля, кнопка входа.
// При успешной аутентификации вызывает App::login(),
// который скрывает это окно и показывает главное.
class LoginWindow : public Fl_Window {
    public:
        explicit LoginWindow(App& app);

        // Вызывается из callback кнопки "Войти".
        // Читает поля, вызывает AuthService::login().
        // При успехе - App::login(), при неудаче - fl_alert с ошибкой.
        void attemptLogin();

        // Очищает поля логина и пароля.
        // Вызывается из App::logout() при возврате на экран входа.
        void clearFields();

    private:
        App& app_;
        Fl_Input*   input_login_  = nullptr;   // поле ввода логина
        Fl_Input*   input_passwd_ = nullptr;   // поле ввода пароля (маскированное)
        Fl_Button*  button_login_ = nullptr;   // кнопка "Войти"

    // Статический callback FLTK для кнопки входа.
    // Получает this через data и вызывает attemptLogin().
    static void onLoginClicked(Fl_Widget* w, void* data);
};

// Идентификаторы разделов приложения.
// Используются для переключения рабочей области в MainWindow.
// Значения соответствуют элементам меню верхнего уровня.
enum class PanelType {
    None,
    Bands,
    Musicians,
    Venues,
    Concerts,
    Contracts,
    Reports,
    Users
};

// Главное окно приложения: верхнее меню и рабочая область.
// В рабочей области размещаются панели (BandPanel, MusicianPanel и т.д.),
// переключаемые пунктами меню.
class MainWindow : public Fl_Window {
    public:
        explicit MainWindow(App& app);

        // Вызывается из callback меню "Файл -> Выход".
        // Делегирует логику App::logout().
        void doLogout();

        // Переключает рабочую область на панель указанного типа.
        // Удаляет предыдущий виджет из workspace_, создаёт новый
        // и добавляет его в рабочую область.
        void showPanel(PanelType type);

        // Возвращает рабочую область в исходное состояние:
        // сбрасывает панель на заглушку, снимает выделение в меню.
        // Вызывается из App::logout() при смене пользователя.
        void reset();

    private:
        App& app_;
        PanelType current_panel_ = PanelType::None;   // тип текущей открытой панели

        Fl_Menu_Bar* menu_           = nullptr;   // верхнее меню
        Fl_Group*    workspace_      = nullptr;   // рабочая область под меню
        Fl_Widget*   current_widget_ = nullptr;   // текущее содержимое рабочей области

        // Статический callback для пункта меню "Файл -> Выход"
        static void onLogout(Fl_Widget* w, void* data);

        // Статический callback для пунктов меню разделов.
        // Через data получает PanelMenuItem (тип панели + указатель на MainWindow).
        static void onPanelSelected(Fl_Widget* w, void* data);
};
