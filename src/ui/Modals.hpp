#pragma once

#include <FL/Fl_Window.H>
#include <vector>
#include <string>

// Данный заголовочный файл содержит объявления модальных окон
// для создания и редактирования записей всех сущностей.
// Каждое окно следует единому паттерну:
//   - открывается из соответствующей панели,
//   - блокирует ввод в родительские окна (set_modal),
//   - после закрытия сообщает панели, сохранились ли данные (saved()).

class Fl_Input;
class Fl_Multiline_Input;
class Fl_Button;
class Fl_Choice;
class App;

namespace models { 
    struct Band;
    struct Musician;
    struct Venue;
    struct Concert;
    struct Contract;
    struct User;
}

// ============================================================
// BandEditDialog - окно создания/редактирования группы
// ============================================================

// Модальное окно формы группы.
// Если передан band == nullptr — режим создания, поля пустые.
// Если band != nullptr — режим редактирования, поля заполнены.
class BandEditDialog : public Fl_Window {
    public:
        BandEditDialog(App& app, const models::Band* band);

        // true, если пользователь нажал "Сохранить" и данные записаны в БД
        bool saved() const { return saved_; }

    private:
        App& app_;
        bool saved_   = false;   // флаг: сохранено ли
        int  band_id_ = 0;       // 0 — создание, >0 — редактирование

        Fl_Input*           input_name_  = nullptr;   // поле "Название"
        Fl_Input*           input_genre_ = nullptr;   // поле "Жанр"
        Fl_Multiline_Input* input_desc_  = nullptr;   // поле "Описание" (многострочное)
        Fl_Button*          btn_save_    = nullptr;   // кнопка "Сохранить"
        Fl_Button*          btn_cancel_  = nullptr;   // кнопка "Отмена"

        // Проверяет, что обязательные поля заполнены.
        // При ошибке показывает alert и возвращает false.
        bool validate();

        // Сохраняет данные: createBand при создании, update при редактировании.
        void doSave();

        static void onSave(Fl_Widget*, void*);
        static void onCancel(Fl_Widget*, void*);
};

// ============================================================
// BandDetailsDialog - окно с подробной информацией о группе
// ============================================================

// Read-only окно: показывает поля группы и состав музыкантов.
// Ничего не редактирует, только читает данные через сервисы.
class BandDetailsDialog : public Fl_Window {
    public:
        BandDetailsDialog(App& app, const models::Band& band);

    private:
        App& app_;
        int  band_id_;

        static void onClose(Fl_Widget*, void*);
};

// ============================================================
// MusicianEditDialog - окно создания/редактирования музыканта
// ============================================================

// Модальное окно формы музыканта.
// Если передан musician == nullptr — режим создания.
// Если musician != nullptr — режим редактирования.
// Группа выбирается из выпадающего списка Fl_Choice;
// соответствие "индекс списка -> id группы" хранится в band_ids_.
class MusicianEditDialog : public Fl_Window {
    public:
        MusicianEditDialog(App& app, const models::Musician* musician);
        bool saved() const { return saved_; }

    private:
        App& app_;
        bool saved_       = false;
        int  musician_id_ = 0;   // 0 — создание, >0 — редактирование

        Fl_Input*  input_name_  = nullptr;    // ФИО
        Fl_Input*  input_phone_ = nullptr;    // телефон
        Fl_Input*  input_email_ = nullptr;    // email
        Fl_Input*  input_instr_ = nullptr;    // инструмент
        Fl_Choice* choice_band_ = nullptr;    // выбор группы (band_id)
        Fl_Button* btn_save_       = nullptr; // кнопка "Сохранить"
        Fl_Button* btn_cancel_     = nullptr; // кнопка "Отмена"

        std::vector<int> band_ids_;          // параллельный массив id групп

        bool validate();
        void doSave();

        // Заполняет choice_band_ списком групп.
        // preselected_id — id, который нужно выделить (для режима редактирования).
        void loadBands(int preselected_id);

        static void onSave(Fl_Widget*, void*);
        static void onCancel(Fl_Widget*, void*);
};

// ============================================================
// VenueEditDialog - окно создания/редактирования площадки
// ============================================================

// Модальное окно формы площадки.
// Если передан venue == nullptr — режим создания.
// Если venue != nullptr — режим редактирования.
class VenueEditDialog : public Fl_Window {
    public:
        VenueEditDialog(App& app, const models::Venue* venue);

        // true, если пользователь нажал "Сохранить" и данные записаны в БД
        bool saved() const { return saved_; }

    private:
        App& app_;
        bool saved_    = false;   // флаг: сохранено ли
        int  venue_id_ = 0;       // 0 — создание, >0 — редактирование

        Fl_Input*  input_name_     = nullptr;   // название
        Fl_Input*  input_address_  = nullptr;   // адрес
        Fl_Input*  input_capacity_ = nullptr;   // вместимость (число)
        Fl_Input*  input_contact_  = nullptr;   // контакт
        Fl_Button* btn_save_       = nullptr;   // кнопка "Сохранить"
        Fl_Button* btn_cancel_     = nullptr;   // кнопка "Отмена"

        bool validate();
        void doSave();

        static void onSave(Fl_Widget*, void*);
        static void onCancel(Fl_Widget*, void*);
};

// ============================================================
// ConcertEditDialog - окно создания/редактирования концерта
// ============================================================

// Модальное окно формы концерта.
// Если передан concert == nullptr — режим создания.
// Если concert != nullptr — режим редактирования.
// Концерт имеет два внешних ключа (band_id, venue_id) —
// оба выбираются через Fl_Choice, id хранятся в параллельных массивах.
class ConcertEditDialog : public Fl_Window {
    public:
        ConcertEditDialog(App& app, const models::Concert* concert);
        bool saved() const { return saved_; }

    private:
        App& app_;
        bool saved_      = false;
        int  concert_id_ = 0;   // 0 — создание, >0 — редактирование

        Fl_Choice* choice_band_    = nullptr;   // выбор группы
        Fl_Choice* choice_venue_   = nullptr;   // выбор площадки
        Fl_Input*  input_date_     = nullptr;   // дата в формате "YYYY-MM-DD"
        Fl_Input*  input_fee_      = nullptr;   // гонорар (число)
        Fl_Input*  input_expenses_ = nullptr;   // расходы (число)
        Fl_Choice* choice_status_  = nullptr;   // planned / done / cancelled
        Fl_Button* btn_save_       = nullptr;   // кнопка "Сохранить"
        Fl_Button* btn_cancel_     = nullptr;   // кнопка "Отмена"

        std::vector<int> band_ids_;    // параллельный массив id групп
        std::vector<int> venue_ids_;   // параллельный массив id площадок

        bool validate();
        void doSave();

        // Заполняют соответствующие Fl_Choice списками.
        // preselected_id — id для выделения (в режиме редактирования).
        void loadBands(int preselected_id);
        void loadVenues(int preselected_id);

        // Заполняет choice_status_ фиксированным списком статусов.
        // current — текущий статус (для выделения).
        void loadStatuses(const std::string& current);

        static void onSave(Fl_Widget*, void*);
        static void onCancel(Fl_Widget*, void*);
};

// ============================================================
// ContractEditDialog - окно создания/редактирования договора
// ============================================================

// Модальное окно формы договора.
// В отличие от остальных диалогов, contract не выбирается:
// concert_id передаётся в конструктор
// Если передан contract == nullptr — режим создания.
// Если contract != nullptr — режим редактирования.
class ContractEditDialog : public Fl_Window {
    public:
        ContractEditDialog(App& app, int concert_id, const models::Contract* contract);
        bool saved() const { return saved_; }

    private:
        App& app_;
        int  concert_id_;         // id концерта (всегда известен из контекста)
        int  contract_id_ = 0;    // 0 — создание, >0 — редактирование
        bool saved_ = false;

        Fl_Input*  input_number_  = nullptr;   // номер договора
        Fl_Input*  input_date_    = nullptr;   // дата "YYYY-MM-DD"
        Fl_Input*  input_amount_  = nullptr;   // сумма (число)
        Fl_Choice* choice_status_ = nullptr;   // draft / signed / paid / cancelled
        Fl_Button* btn_save_      = nullptr;   // кнопка "Сохранить"
        Fl_Button* btn_cancel_    = nullptr;   // кнопка "Отмена"

        bool validate();
        void doSave();

        // Заполняет choice_status_ фиксированным списком статусов.
        void loadStatuses(const std::string& current);

        static void onSave(Fl_Widget*, void*);
        static void onCancel(Fl_Widget*, void*);
};

// ============================================================
// UserCreateDialog - окно создания пользователя
// ============================================================

// Модальное окно создания пользователя.
// Только создание — редактирование пользователей не предусмотрено
// Пароль хешируется при сохранении через AuthService::createUser.
// Роль выбирается из Fl_Choice; соответствие "индекс -> id роли"
// хранится в role_ids_.
class UserCreateDialog : public Fl_Window {
    public:
        UserCreateDialog(App& app);
        bool saved() const { return saved_; }

    private:
        App& app_;
        bool saved_ = false;

        Fl_Input*  input_login_    = nullptr;   // логин
        Fl_Input*  input_password_ = nullptr;   // пароль (FL_SECRET_INPUT)
        Fl_Choice* choice_role_    = nullptr;   // выбор роли

        std::vector<int> role_ids_;             // параллельный массив id ролей

        Fl_Button* btn_save_   = nullptr;   // кнопка "Сохранить"
        Fl_Button* btn_cancel_ = nullptr;   // кнопка "Отмена"

        // Заполняет choice_role_ списком ролей из AuthService::getAllRoles().
        void loadRoles();

        bool validate();
        void doSave();

        static void onSave(Fl_Widget*, void*);
        static void onCancel(Fl_Widget*, void*);
};
