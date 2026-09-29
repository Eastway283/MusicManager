#include "ui/Modals.hpp"
#include "ui/App.hpp"
#include "Models.hpp"
#include "services/CatalogService.hpp"
#include "db/Database.hpp"
#include "util/Status.hpp"

#include <FL/Fl_Input.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Hold_Browser.H>

#include <string>

// ============================================================
// BandEditDialog - окно создания/редактирования группы
// ============================================================

// Конструктор: создаёт поля ввода и кнопки.
// Если band != nullptr — режим редактирования, поля заполняются
// данными группы, запоминается band_id_ для последующего update.
// Если band == nullptr — режим создания, поля пустые.
BandEditDialog::BandEditDialog(App& app, const models::Band* band)
    : Fl_Window(420, 260, band ? "Изменить группу" : "Создать группу"),
      app_(app)
{
    // Если редактируем — запоминаем id
    if (band) band_id_ = band->id;

    // Координаты полей: x — отступ слева, y — текущая позиция по вертикали,
    // dy — шаг между полями
    int x  = 10;
    int y  = 15;
    int dy = 35;

    // Поле "Название"
    input_name_ = new Fl_Input(x + 100, y, 300, 25, "Название:");
    input_name_->align(FL_ALIGN_LEFT);
    y += dy;

    // Поле "Жанр"
    input_genre_ = new Fl_Input(x + 100, y, 300, 25, "Жанр:");
    input_genre_->align(FL_ALIGN_LEFT);
    y += dy;

    // Поле "Описание" — многострочное, высота 60 пикселей
    input_desc_ = new Fl_Multiline_Input(x + 100, y, 300, 60, "Описание:");
    input_desc_->align(FL_ALIGN_LEFT);
    y += 70;

    // Кнопки
    btn_save_   = new Fl_Button(x + 200, y, 100, 30, "Сохранить");
    btn_cancel_ = new Fl_Button(x + 310, y, 100, 30, "Отмена");
    btn_save_->callback(onSave, this);
    btn_cancel_->callback(onCancel, this);

    // Заполнение полей, если редактируем
    if (band) {
        input_name_->value(band->name.c_str());
        input_genre_->value(band->genre.c_str());
        input_desc_->value(band->description.c_str());
    }

    end();         // закрываем окно: виджеты больше не добавляются
    set_modal();   // блокирует ввод в родительские окна
}

// Проверяет обязательные поля.
// При ошибке показывает alert и возвращает false.
bool BandEditDialog::validate() {
    if (!input_name_->value() || std::string(input_name_->value()).empty()) {
        fl_alert("Введите название группы");
        return false;
    }
    if (!input_genre_->value() || std::string(input_genre_->value()).empty()) {
        fl_alert("Введите жанр");
        return false;
    }
    return true;
}

// Сохраняет данные:
//   - при создании (band_id_ == 0) — createBand,
//     менеджером становится текущий пользователь;
//   - при редактировании — update с новыми значениями полей.
// После успешного сохранения ставит saved_ = true и скрывает окно.
void BandEditDialog::doSave() {
    if (!validate()) return;

    std::string name  = input_name_->value()  ? input_name_->value()  : "";
    std::string genre = input_genre_->value() ? input_genre_->value() : "";
    std::string desc  = input_desc_->value()  ? input_desc_->value()  : "";

    try {
        if (band_id_ == 0) {
            // Создание: менеджером становится текущий пользователь
            int manager_id = app_.currentUser().id;
            app_.bands().createBand(name, genre, desc, manager_id);
        } else {
            // Редактирование: подгружаем существующую запись,
            // меняем поля и сохраняем
            auto existing = app_.bands().getById(band_id_);
            if (existing) {
                existing->name        = name;
                existing->genre       = genre;
                existing->description = desc;
                app_.bands().update(*existing);
            }
        }
        saved_ = true;
        hide();
    } catch (const DbError& e) {
        fl_alert("Ошибка сохранения: %s", e.what());
    }
}

// Статический callback кнопки "Сохранить".
// Получает this через data и вызывает doSave().
void BandEditDialog::onSave(Fl_Widget*, void* data) {
    static_cast<BandEditDialog*>(data)->doSave();
}

// Статический callback кнопки "Отмена".
// Просто скрывает окно, saved_ остаётся false.
void BandEditDialog::onCancel(Fl_Widget*, void* data) {
    static_cast<BandEditDialog*>(data)->hide();
}

// ============================================================
// BandDetailsDialog - окно с подробной информацией о группе
// ============================================================

// Конструктор: создаёт статичные поля (Fl_Box) с информацией о группе
// и список музыкантов, входящих в состав.
// Все поля read-only, редактирование невозможно.
BandDetailsDialog::BandDetailsDialog(App& app, const models::Band& band)
    : Fl_Window(520, 460, "Информация о группе"),
      app_(app),
      band_id_(band.id)
{
    int x = 15;
    int y = 15;
    int dy = 25;

    // --- Основная информация ---

    // Название — крупным шрифтом
    auto* title = new Fl_Box(x, y, 490, 30, band.name.c_str());
    title->labelsize(20);
    title->labelfont(FL_BOLD);
    title->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    y += dy + 15;

    // Жанр
    std::string genre_text = "Жанр: " + band.genre;
    auto* genre = new Fl_Box(x, y, 490, 22);
    genre->copy_label(genre_text.c_str());
    genre->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    y += dy;

    // Описание — многострочный, с переносом
    std::string desc_text = "Описание: " + band.description;
    auto* desc = new Fl_Box(x, y, 490, 60);
    desc->copy_label(desc_text.c_str());
    desc->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    y += 70;

    // --- Разделитель ---
    auto* sep = new Fl_Box(x, y, 490, 2);
    sep->box(FL_FLAT_BOX);
    sep->color(FL_DARK3);
    y += 15;

    // --- Состав группы ---
    auto* label = new Fl_Box(x, y, 490, 22, "Состав группы:");
    label->labelfont(FL_BOLD);
    label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    y += dy;

    // Список музыкантов — read-only Fl_Hold_Browser
    auto* browser = new Fl_Hold_Browser(x, y, 490, 260);
    auto musicians = app_.musicians().findByBand(band_id_);
    if (musicians.empty()) {
        browser->add("(состав пуст)");
    } else {
        for (auto& m : musicians) {
            std::string line = m.name;
            if (!m.instrument.empty())
                line += " — " + m.instrument;
            browser->add(line.c_str());
        }
    }
    y += 270;

    // --- Кнопка закрытия ---
    auto* btn_close = new Fl_Button(410, y + 5, 100, 30, "Закрыть");
    btn_close->callback(onClose, this);

    end();
    set_modal();
}

void BandDetailsDialog::onClose(Fl_Widget*, void* data) {
    static_cast<BandDetailsDialog*>(data)->hide();
}

// ============================================================
// MusicianEditDialog - окно создания/редактирования музыканта
// ============================================================

// Конструктор: создаёт поля ввода, выпадающий список групп и кнопки.
// Если musician != nullptr — режим редактирования: поля заполняются
// данными музыканта, в списке групп выделяется его группа.
// Если musician == nullptr — режим создания, поля пустые.
MusicianEditDialog::MusicianEditDialog(App& app, const models::Musician* musician)
    : Fl_Window(420, 300, musician ? "Изменить музыканта" : "Создать музыканта"),
      app_(app)
{
    // Если редактируем — запоминаем id
    if (musician) musician_id_ = musician->id;

    // Координаты полей: x — отступ слева, y — текущая позиция по вертикали,
    // dy — шаг между полями
    int x  = 10;
    int y  = 15;
    int dy = 35;

    // Поле "Имя"
    input_name_ = new Fl_Input(x + 100, y, 300, 25, "Имя:");
    input_name_->align(FL_ALIGN_LEFT);
    y += dy;

    // Поле "Телефон"
    input_phone_ = new Fl_Input(x + 100, y, 300, 25, "Телефон:");
    input_phone_->align(FL_ALIGN_LEFT);
    y += dy;

    // Поле "Email"
    input_email_ = new Fl_Input(x + 100, y, 300, 25, "Email:");
    input_email_->align(FL_ALIGN_LEFT);
    y += dy;

    // Поле "Инструмент"
    input_instr_ = new Fl_Input(x + 100, y, 300, 25, "Инструмент:");
    input_instr_->align(FL_ALIGN_LEFT);
    y += dy;

    // Выпадающий список групп
    choice_band_ = new Fl_Choice(x + 100, y, 300, 25, "Группа:");
    choice_band_->align(FL_ALIGN_LEFT);
    y += dy;

    // Кнопки
    btn_save_   = new Fl_Button(x + 200, y, 100, 30, "Сохранить");
    btn_cancel_ = new Fl_Button(x + 310, y, 100, 30, "Отмена");
    btn_save_->callback(onSave, this);
    btn_cancel_->callback(onCancel, this);

    // Заполняем список групп. Если редактируем — выделяем группу музыканта.
    loadBands(musician ? musician->band_id : 0);

    // Заполнение остальных полей, если редактируем
    if (musician) {
        input_name_->value(musician->name.c_str());
        input_phone_->value(musician->phone.c_str());
        input_email_->value(musician->email.c_str());
        input_instr_->value(musician->instrument.c_str());
    }

    end();         // закрываем окно: виджеты больше не добавляются
    set_modal();   // блокирует ввод в родительские окна
}

// Заполняет choice_band_ списком групп из БД.
// Параллельно заполняет band_ids_ — массив id, где band_ids_[i] —
// id группы, показанной в choice_band_ под индексом i.
// preselected_id — id группы, которую нужно выделить в списке
// (в режиме редактирования; 0 — если ничего выделять не надо).
void MusicianEditDialog::loadBands(int preselected_id) {
    auto bands = app_.bands().getAll();
    int selected = 0;

    for (size_t i = 0; i < bands.size(); ++i) {
        choice_band_->add(bands[i].name.c_str());
        band_ids_.push_back(bands[i].id);
        if (bands[i].id == preselected_id)
            selected = static_cast<int>(i);
    }
    choice_band_->value(selected);
}

// Проверяет обязательные поля.
// Имя обязательно (NOT NULL в БД), группа обязательна (band_id NOT NULL).
// Телефон, email, инструмент — опциональны, могут быть пустыми.
bool MusicianEditDialog::validate() {
    if (!input_name_->value() || std::string(input_name_->value()).empty()) {
        fl_alert("Введите имя музыканта");
        return false;
    }
    if (choice_band_->value() < 0) {
        fl_alert("Выберите группу");
        return false;
    }
    return true;
}

// Сохраняет данные:
//   - при создании (musician_id_ == 0) — createMusician;
//   - при редактировании — update с новыми значениями полей.
// После успешного сохранения ставит saved_ = true и скрывает окно.
void MusicianEditDialog::doSave() {
    if (!validate()) return;

    std::string name       = input_name_->value()  ? input_name_->value()  : "";
    std::string phone      = input_phone_->value() ? input_phone_->value() : "";
    std::string email      = input_email_->value() ? input_email_->value() : "";
    std::string instrument = input_instr_->value() ? input_instr_->value() : "";

    // id выбранной группы: по индексу в choice_band_ берём из параллельного массива
    int band_idx = choice_band_->value();
    int band_id  = band_ids_[band_idx];

    try {
        if (musician_id_ == 0) {
            // Создание
            app_.musicians().createMusician(name, phone, email, instrument, band_id);
        } else {
            // Редактирование: подгружаем существующую запись,
            // меняем поля и сохраняем
            auto existing = app_.musicians().getById(musician_id_);
            if (existing) {
                existing->name       = name;
                existing->phone      = phone;
                existing->email      = email;
                existing->instrument = instrument;
                existing->band_id    = band_id;
                app_.musicians().update(*existing);
            }
        }
        saved_ = true;
        hide();
    } catch (const DbError& e) {
        fl_alert("Ошибка сохранения: %s", e.what());
    }
}

// Статический callback кнопки "Сохранить".
// Получает this через data и вызывает doSave().
void MusicianEditDialog::onSave(Fl_Widget*, void* data) {
    static_cast<MusicianEditDialog*>(data)->doSave();
}

// Статический callback кнопки "Отмена".
// Просто скрывает окно, saved_ остаётся false.
void MusicianEditDialog::onCancel(Fl_Widget*, void* data) {
    static_cast<MusicianEditDialog*>(data)->hide();
}

// ============================================================
// VenueEditDialog - окно создания/редактирования площадки
// ============================================================

// Конструктор: создаёт поля ввода и кнопки.
// Если venue != nullptr — режим редактирования, поля заполняются
// данными площадки, запоминается venue_id_ для последующего update.
// Если venue == nullptr — режим создания, поля пустые.
VenueEditDialog::VenueEditDialog(App& app, const models::Venue* venue)
    : Fl_Window(420, 260, venue ? "Изменить площадку" : "Создать площадку"),
      app_(app)
{
    // Если редактируем — запоминаем id
    if (venue) venue_id_ = venue->id;

    // Координаты полей: x — отступ слева, y — текущая позиция по вертикали,
    // dy — шаг между полями
    int x  = 10;
    int y  = 15;
    int dy = 35;

    // Поле "Название"
    input_name_ = new Fl_Input(x + 100, y, 300, 25, "Название:");
    input_name_->align(FL_ALIGN_LEFT);
    y += dy;

    // Поле "Адрес"
    input_address_ = new Fl_Input(x + 100, y, 300, 25, "Адрес:");
    input_address_->align(FL_ALIGN_LEFT);
    y += dy;

    // Поле "Вместимость"
    input_capacity_ = new Fl_Input(x + 100, y, 300, 25, "Вместимость:");
    input_capacity_->align(FL_ALIGN_LEFT);
    y += dy;

    // Поле "Контакт"
    input_contact_ = new Fl_Input(x + 100, y, 300, 25, "Контакт: ");
    input_contact_->align(FL_ALIGN_LEFT);
    y += dy;

    // Кнопки
    btn_save_   = new Fl_Button(x + 200, y, 100, 30, "Сохранить");
    btn_cancel_ = new Fl_Button(x + 310, y, 100, 30, "Отмена");
    btn_save_->callback(onSave, this);
    btn_cancel_->callback(onCancel, this);

    // Заполнение полей, если редактируем
    if (venue) {
        input_name_->value(venue->name.c_str());
        input_address_->value(venue->address.c_str());
        std::string cap_str = std::to_string(venue->capacity);
        input_capacity_->value(cap_str.c_str());
        input_contact_->value(venue->contact.c_str());
    }

    end();         // закрываем окно: виджеты больше не добавляются
    set_modal();   // блокирует ввод в родительские окна
}

// Проверяет обязательные поля.
// При ошибке показывает alert и возвращает false.
bool VenueEditDialog::validate() {
    if (!input_name_->value() || std::string(input_name_->value()).empty()) {
        fl_alert("Введите название площадки");
        return false;
    }
    if (!input_address_->value() || std::string(input_address_->value()).empty()) {
        fl_alert("Введите адрес");
        return false;
    }
    if (!input_capacity_->value() || std::string(input_capacity_->value()).empty()) {
        fl_alert("Введите вместимость");
        return false;
    }
    if (!input_contact_->value() || std::string(input_contact_->value()).empty()) {
        fl_alert("Введите контакт");
        return false;
    }
    return true;
}

// Сохраняет данные:
//   - при создании (venue_id_ == 0) — createVenue,
//     менеджером становится текущий пользователь;
//   - при редактировании — update с новыми значениями полей.
// После успешного сохранения ставит saved_ = true и скрывает окно.
void VenueEditDialog::doSave() {
    if (!validate()) return;

    std::string name    = input_name_->value()    ? input_name_->value()    : "";
    std::string address = input_address_->value() ? input_address_->value() : "";
    std::string contact = input_contact_->value() ? input_contact_->value() : "";

    // Преобразование вместимости из строки в int
    int capacity = 0;
    try {
        capacity = std::stoi(input_capacity_->value());
    } catch (const std::exception&) {
        fl_alert("Вместимость должна быть числом");
        return;
    }

    try {
        if (venue_id_ == 0) {
            // Создание
            app_.venues().createVenue(name, address, capacity, contact);
        } else {
            // Редактирование
            auto existing = app_.venues().getById(venue_id_);
            if (existing) {
                existing->name     = name;
                existing->address  = address;
                existing->capacity = capacity;
                existing->contact  = contact;
                app_.venues().update(*existing);
            }
        }
        saved_ = true;
        hide();
    } catch (const DbError& e) {
        fl_alert("Ошибка сохранения: %s", e.what());
    }
}

// Статический callback кнопки "Сохранить".
// Получает this через data и вызывает doSave().
void VenueEditDialog::onSave(Fl_Widget*, void* data) {
    static_cast<VenueEditDialog*>(data)->doSave();
}

// Статический callback кнопки "Отмена".
// Просто скрывает окно, saved_ остаётся false.
void VenueEditDialog::onCancel(Fl_Widget*, void* data) {
    static_cast<VenueEditDialog*>(data)->hide();
}

// ============================================================
// ConcertEditDialog - окно создания/редактирования концерта
// ============================================================

// Конструктор: создаёт два выпадающих списка (группа, площадка),
// поле даты, два числовых поля (гонорар, расходы),
// выпадающий список статуса и кнопки.
// Если concert != nullptr — режим редактирования: поля заполняются
// данными концерта, в списках выделяются его группа и площадка.
// Если concert == nullptr — режим создания, поля пустые.
ConcertEditDialog::ConcertEditDialog(App& app, const models::Concert* concert)
    : Fl_Window(420, 320, concert ? "Изменить концерт" : "Создать концерт"),
      app_(app)
{
    // Если редактируем — запоминаем id
    if (concert) concert_id_ = concert->id;

    // Координаты полей: x — отступ слева, y — текущая позиция по вертикали,
    // dy — шаг между полями
    int x  = 10;
    int y  = 15;
    int dy = 35;

    // Выпадающий список групп
    choice_band_ = new Fl_Choice(x + 110, y, 290, 25, "Группа:");
    choice_band_->align(FL_ALIGN_LEFT);
    y += dy;

    // Выпадающий список площадок
    choice_venue_ = new Fl_Choice(x + 110, y, 290, 25, "Площадка:");
    choice_venue_->align(FL_ALIGN_LEFT);
    y += dy;

    // Поле "Дата" в формате YYYY-MM-DD
    input_date_ = new Fl_Input(x + 110, y, 290, 25, "Дата:");
    input_date_->align(FL_ALIGN_LEFT);
    y += dy;

    // Поле "Гонорар" (число)
    input_fee_ = new Fl_Input(x + 110, y, 290, 25, "Гонорар:");
    input_fee_->align(FL_ALIGN_LEFT);
    y += dy;

    // Поле "Расходы" (число)
    input_expenses_ = new Fl_Input(x + 110, y, 290, 25, "Расходы:");
    input_expenses_->align(FL_ALIGN_LEFT);
    y += dy;

    // Выпадающий список статуса (фиксированный набор)
    choice_status_ = new Fl_Choice(x + 110, y, 290, 25, "Статус:");
    choice_status_->align(FL_ALIGN_LEFT);
    y += dy + 5;

    // Кнопки
    btn_save_   = new Fl_Button(x + 200, y, 100, 30, "Сохранить");
    btn_cancel_ = new Fl_Button(x + 310, y, 100, 30, "Отмена");
    btn_save_->callback(onSave, this);
    btn_cancel_->callback(onCancel, this);

    // Заполняем списки. При редактировании выделяем текущие значения.
    loadBands(concert ? concert->band_id : 0);
    loadVenues(concert ? concert->venue_id : 0);
    loadStatuses(concert ? concert->status : "planned");

    // Заполнение остальных полей, если редактируем
    if (concert) {
        input_date_->value(concert->date.c_str());
        input_fee_->value(std::to_string(concert->fee).c_str());
        input_expenses_->value(std::to_string(concert->expenses).c_str());
    }

    end();         // закрываем окно: виджеты больше не добавляются
    set_modal();   // блокирует ввод в родительские окна
}

// Заполняет choice_band_ списком групп из БД.
// Параллельно заполняет band_ids_ — массив id, где band_ids_[i] —
// id группы, показанной в choice_band_ под индексом i.
void ConcertEditDialog::loadBands(int preselected_id) {
    auto bands = app_.bands().getAll();
    int selected = 0;

    for (size_t i = 0; i < bands.size(); ++i) {
        choice_band_->add(bands[i].name.c_str());
        band_ids_.push_back(bands[i].id);
        if (bands[i].id == preselected_id)
            selected = static_cast<int>(i);
    }
    choice_band_->value(selected);
}

// Заполняет choice_venue_ списком площадок из БД.
// Параллельно заполняет venue_ids_.
void ConcertEditDialog::loadVenues(int preselected_id) {
    auto venues = app_.venues().getAll();
    int selected = 0;

    for (size_t i = 0; i < venues.size(); ++i) {
        choice_venue_->add(venues[i].name.c_str());
        venue_ids_.push_back(venues[i].id);
        if (venues[i].id == preselected_id)
            selected = static_cast<int>(i);
    }
    choice_venue_->value(selected);
}

// Заполняет choice_status_ фиксированным списком статусов.
// current — текущий статус (для выделения при редактировании).
// Соответствие индексов и значений хранится прямо здесь,
// потому что список никогда не меняется.
void ConcertEditDialog::loadStatuses(const std::string& current) {
    choice_status_->add("Запланирован");
    choice_status_->add("Проведён");
    choice_status_->add("Отменён");

    if (current == "planned")        choice_status_->value(0);
    else if (current == "done")      choice_status_->value(1);
    else if (current == "cancelled") choice_status_->value(2);
    else                             choice_status_->value(0);
}

// Проверяет обязательные поля.
// Группа и площадка обязательны (NOT NULL в БД),
// дата обязательна, статус всегда выбран.
// Числа проверяются в doSave через std::stod с try/catch.
bool ConcertEditDialog::validate() {
    if (choice_band_->value() < 0) {
        fl_alert("Выберите группу. Если список пуст — сначала создайте группу.");
        return false;
    }
    if (choice_venue_->value() < 0) {
        fl_alert("Выберите площадку. Если список пуст — сначала создайте площадку.");
        return false;
    }
    if (!input_date_->value() || std::string(input_date_->value()).empty()) {
        fl_alert("Введите дату в формате YYYY-MM-DD");
        return false;
    }
    return true;
}

// Сохраняет данные:
//   - при создании (concert_id_ == 0) — createConcert;
//   - при редактировании — update с новыми значениями полей.
// Числовые поля конвертируются из строк через std::stod с обработкой
// исключений: если пользователь ввёл не число, покажем alert и выйдем.
void ConcertEditDialog::doSave() {
    if (!validate()) return;

    // id выбранной группы и площадки — по индексу из параллельных массивов
    int band_id  = band_ids_[choice_band_->value()];
    int venue_id = venue_ids_[choice_venue_->value()];

    std::string date   = input_date_->value() ? input_date_->value() : "";
    std::string ru_status = choice_status_->text() ? choice_status_->text() : "Запланирован";
    std::string status = concertStatusFromRu(ru_status);

    // Преобразование чисел. Если поле пустое — 0.
    double fee = 0.0;
    double expenses = 0.0;
    try {
        if (input_fee_->value() && *input_fee_->value())
            fee = std::stod(input_fee_->value());
        if (input_expenses_->value() && *input_expenses_->value())
            expenses = std::stod(input_expenses_->value());
    } catch (const std::exception&) {
        fl_alert("Гонорар и расходы должны быть числами");
        return;
    }

    try {
        if (concert_id_ == 0) {
            // Создание
            app_.concerts().createConcert(band_id, venue_id, date, fee, expenses);
        } else {
            // Редактирование: подгружаем существующую запись,
            // меняем поля и сохраняем
            auto existing = app_.concerts().getById(concert_id_);
            if (existing) {
                existing->band_id  = band_id;
                existing->venue_id = venue_id;
                existing->date     = date;
                existing->fee      = fee;
                existing->expenses = expenses;
                existing->status   = status;
                app_.concerts().update(*existing);
            }
        }
        saved_ = true;
        hide();
    } catch (const DbError& e) {
        fl_alert("Ошибка сохранения: %s", e.what());
    }
}

// Статический callback кнопки "Сохранить".
void ConcertEditDialog::onSave(Fl_Widget*, void* data) {
    static_cast<ConcertEditDialog*>(data)->doSave();
}

// Статический callback кнопки "Отмена".
void ConcertEditDialog::onCancel(Fl_Widget*, void* data) {
    static_cast<ConcertEditDialog*>(data)->hide();
}

// ============================================================
// ContractEditDialog - окно создания/редактирования договора
// ============================================================

// Конструктор: создаёт поля ввода, выпадающий список статуса и кнопки.
// concert_id приходит извне (открывается из ConcertPanel),
// поэтому выбора концерта в форме нет — связь 1:1.
// Если contract != nullptr — режим редактирования: поля заполняются
// данными договора, в списке статусов выделяется текущий.
// Если contract == nullptr — режим создания, поля пустые.
ContractEditDialog::ContractEditDialog(App& app, int concert_id,
                                       const models::Contract* contract)
    : Fl_Window(420, 240, contract ? "Изменить договор" : "Создать договор"),
      app_(app),
      concert_id_(concert_id)
{
    // Если редактируем — запоминаем id
    if (contract) contract_id_ = contract->id;

    // Координаты полей: x — отступ слева, y — текущая позиция по вертикали,
    // dy — шаг между полями
    int x  = 10;
    int y  = 15;
    int dy = 35;

    // Поле "Номер"
    input_number_ = new Fl_Input(x + 110, y, 290, 25, "Номер:");
    input_number_->align(FL_ALIGN_LEFT);
    y += dy;

    // Поле "Дата" в формате YYYY-MM-DD
    input_date_ = new Fl_Input(x + 110, y, 290, 25, "Дата:");
    input_date_->align(FL_ALIGN_LEFT);
    y += dy;

    // Поле "Сумма" (число)
    input_amount_ = new Fl_Input(x + 110, y, 290, 25, "Сумма:");
    input_amount_->align(FL_ALIGN_LEFT);
    y += dy;

    // Выпадающий список статуса (фиксированный набор)
    choice_status_ = new Fl_Choice(x + 110, y, 290, 25, "Статус:");
    choice_status_->align(FL_ALIGN_LEFT);
    y += dy + 5;

    // Кнопки
    btn_save_   = new Fl_Button(x + 200, y, 100, 30, "Сохранить");
    btn_cancel_ = new Fl_Button(x + 310, y, 100, 30, "Отмена");
    btn_save_->callback(onSave, this);
    btn_cancel_->callback(onCancel, this);

    // Заполняем список статусов
    loadStatuses(contract ? contract->status : "draft");

    // Заполнение остальных полей, если редактируем
    if (contract) {
        input_number_->value(contract->number.c_str());
        input_date_->value(contract->date.c_str());
        input_amount_->value(std::to_string(contract->amount).c_str());
    }

    end();         // закрываем окно: виджеты больше не добавляются
    set_modal();   // блокирует ввод в родительские окна
}

// Заполняет choice_status_ фиксированным списком статусов.
// current — текущий статус (для выделения при редактировании).
// Соответствие индексов и значений хранится прямо здесь,
// потому что список никогда не меняется.
void ContractEditDialog::loadStatuses(const std::string& current) {
    choice_status_->add("Черновик");
    choice_status_->add("Подписан");
    choice_status_->add("Оплачен");
    choice_status_->add("Расторгнут");

    if (current == "draft")          choice_status_->value(0);
    else if (current == "signed")    choice_status_->value(1);
    else if (current == "paid")      choice_status_->value(2);
    else if (current == "cancelled") choice_status_->value(3);
    else                              choice_status_->value(0);
}

// Проверяет обязательные поля.
// Номер, дата и сумма обязательны (NOT NULL в БД),
// статус всегда выбран. Числа проверяются в doSave.
bool ContractEditDialog::validate() {
    if (!input_number_->value() || std::string(input_number_->value()).empty()) {
        fl_alert("Введите номер договора");
        return false;
    }
    if (!input_date_->value() || std::string(input_date_->value()).empty()) {
        fl_alert("Введите дату в формате YYYY-MM-DD");
        return false;
    }
    if (!input_amount_->value() || std::string(input_amount_->value()).empty()) {
        fl_alert("Введите сумму");
        return false;
    }
    return true;
}

// Сохраняет данные:
//   - при создании (contract_id_ == 0) — createContract с concert_id_,
//     затем changeStatus, если выбранный статус не 'draft'
//     (createContract всегда ставит 'draft' по умолчанию);
//   - при редактировании — update с новыми значениями полей.
// Числовое поле amount конвертируется через std::stod с обработкой
// исключений: если пользователь ввёл не число, покажем alert и выйдем.
void ContractEditDialog::doSave() {
    if (!validate()) return;

    std::string number = input_number_->value() ? input_number_->value() : "";
    std::string date   = input_date_->value()   ? input_date_->value()   : "";
    std::string ru_status = choice_status_->text() ? choice_status_->text() : "Черновик";
    std::string status = contractStatusFromRu(ru_status);

    // Преобразование суммы из строки в double
    double amount = 0.0;
    try {
        amount = std::stod(input_amount_->value());
    } catch (const std::exception&) {
        fl_alert("Сумма должна быть числом");
        return;
    }

    try {
        if (contract_id_ == 0) {
            // Создание
            int new_id = app_.contracts().createContract(concert_id_, number, date, amount);
            // createContract ставит 'draft' по умолчанию;
            // если пользователь выбрал другой статус — меняем сразу
            if (status != "draft") {
                app_.contracts().changeStatus(new_id, status);
            }
        } else {
            // Редактирование: подгружаем существующую запись,
            // меняем поля и сохраняем
            auto existing = app_.contracts().getById(contract_id_);
            if (existing) {
                existing->number  = number;
                existing->date    = date;
                existing->amount  = amount;
                existing->status  = status;
                app_.contracts().update(*existing);
            }
        }
        saved_ = true;
        hide();
    } catch (const DbError& e) {
        fl_alert("Ошибка сохранения: %s", e.what());
    }
}

// Статический callback кнопки "Сохранить".
void ContractEditDialog::onSave(Fl_Widget*, void* data) {
    static_cast<ContractEditDialog*>(data)->doSave();
}

// Статический callback кнопки "Отмена".
void ContractEditDialog::onCancel(Fl_Widget*, void* data) {
    static_cast<ContractEditDialog*>(data)->hide();
}

// ============================================================
// UserCreateDialog - окно создания пользователя
// ============================================================

// Конструктор: создаёт поля логина, пароля, выпадающий список ролей и кнопки.
// Режим только создания — редактирование пользователей не предусмотрено.
// Пароль маскируется через FL_SECRET_INPUT.
UserCreateDialog::UserCreateDialog(App& app)
    : Fl_Window(420, 200, "Создать пользователя"),
      app_(app)
{
    // Координаты полей: x — отступ слева, y — текущая позиция по вертикали,
    // dy — шаг между полями
    int x  = 10;
    int y  = 15;
    int dy = 35;

    // Поле "Логин"
    input_login_ = new Fl_Input(x + 100, y, 300, 25, "Логин:");
    input_login_->align(FL_ALIGN_LEFT);
    y += dy;

    // Поле "Пароль" с маскировкой ввода
    input_password_ = new Fl_Input(x + 100, y, 300, 25, "Пароль:");
    input_password_->align(FL_ALIGN_LEFT);
    input_password_->type(FL_SECRET_INPUT);
    y += dy;

    // Выпадающий список ролей
    choice_role_ = new Fl_Choice(x + 100, y, 300, 25, "Роль:");
    choice_role_->align(FL_ALIGN_LEFT);
    y += dy + 5;

    // Кнопки
    btn_save_   = new Fl_Button(x + 200, y, 100, 30, "Сохранить");
    btn_cancel_ = new Fl_Button(x + 310, y, 100, 30, "Отмена");
    btn_save_->callback(onSave, this);
    btn_cancel_->callback(onCancel, this);

    // Заполняем список ролей из БД
    loadRoles();

    end();         // закрываем окно: виджеты больше не добавляются
    set_modal();   // блокирует ввод в родительские окна
}

// Заполняет choice_role_ списком ролей из AuthService::getAllRoles().
// Параллельно заполняет role_ids_ — массив id, где role_ids_[i] —
// id роли, показанной в choice_role_ под индексом i.
void UserCreateDialog::loadRoles() {
    auto roles = app_.auth().getAllRoles();
    int selected = 0;

    for (size_t i = 0; i < roles.size(); ++i) {
        choice_role_->add(roles[i].name.c_str());
        role_ids_.push_back(roles[i].id);
        // По умолчанию выделяем manager (id = 2), если он есть в списке.
        // Если роли в БД перепутаны — выделится первая.
        if (roles[i].name == "manager")
            selected = static_cast<int>(i);
    }
    choice_role_->value(selected);
}

// Проверяет обязательные поля.
// Логин и пароль обязательны, роль всегда выбрана (loadRoles ставит индекс).
bool UserCreateDialog::validate() {
    if (!input_login_->value() || std::string(input_login_->value()).empty()) {
        fl_alert("Введите логин");
        return false;
    }
    if (!input_password_->value() || std::string(input_password_->value()).empty()) {
        fl_alert("Введите пароль");
        return false;
    }
    if (choice_role_->value() < 0) {
        fl_alert("Выберите роль. Если список пуст — проверьте таблицу roles.");
        return false;
    }
    return true;
}

// Создаёт пользователя через AuthService::createUser.
// Пароль хешируется с солью внутри createUser — в открытом виде не хранится.
// Если логин уже занят, SQLite бросит DbError (UNIQUE constraint).
void UserCreateDialog::doSave() {
    if (!validate()) return;

    std::string login    = input_login_->value()    ? input_login_->value()    : "";
    std::string password = input_password_->value() ? input_password_->value() : "";

    // id выбранной роли — по индексу из параллельного массива
    int role_id = role_ids_[choice_role_->value()];

    try {
        app_.auth().createUser(login, password, role_id);
        saved_ = true;
        hide();
    } catch (const DbError& e) {
        fl_alert("Ошибка создания: %s", e.what());
    }
}

// Статический callback кнопки "Сохранить".
void UserCreateDialog::onSave(Fl_Widget*, void* data) {
    static_cast<UserCreateDialog*>(data)->doSave();
}

// Статический callback кнопки "Отмена".
void UserCreateDialog::onCancel(Fl_Widget*, void* data) {
    static_cast<UserCreateDialog*>(data)->hide();
}
