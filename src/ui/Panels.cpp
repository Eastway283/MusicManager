#include "ui/Panels.hpp"
#include "ui/App.hpp"
#include "ui/Modals.hpp"

#include "services/AuthService.hpp"
#include "services/CatalogService.hpp"
#include "services/ConcertService.hpp"
#include "services/ContractService.hpp"
#include "db/Database.hpp"
#include "util/Date.hpp"
#include "util/Status.hpp"
#include "Models.hpp"

#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>

#include <cctype>
#include <cstdint>
#include <string>

// ============================================================
// BandPanel - панель управления группами
// ============================================================

// Конструктор панели: создаёт список групп и кнопки управления.
// Координаты x, y, w, h передаются из MainWindow и определяют,
// где панель расположена внутри рабочей области.
BandPanel::BandPanel(int x, int y, int w, int h, App& app)
    : Fl_Group(x, y, w, h),
      app_(app)
{
    // Список групп — верхняя часть, снизу 70 пикселей под кнопки
    list_ = new Fl_Hold_Browser(x + 10, y + 10, w - 20, h - 70);

    // Кнопки управления
    int by = y + h - 50;
    btn_add_     = new Fl_Button(x + 10,  by, 100, 30, "Добавить");
    btn_edit_    = new Fl_Button(x + 120, by, 100, 30, "Изменить");
    btn_delete_  = new Fl_Button(x + 230, by, 100, 30, "Удалить");
    btn_refresh_ = new Fl_Button(x + 340, by, 100, 30, "Обновить");
    btn_details_ = new Fl_Button(x + 450, by, 100, 30, "Подробнее");

    btn_add_->callback(onAdd, this);
    btn_edit_->callback(onEdit, this);
    btn_delete_->callback(onDelete, this);
    btn_refresh_->callback(onRefresh, this);
    btn_details_->callback(onDetails, this);

    end();
    refresh();
}

// Перечитывает все группы из БД и заново заполняет список.
void BandPanel::refresh() {
    list_->clear();
    auto bands = app_.bands().getAll();
    for (auto& b : bands) {
        std::string label = b.name + " (" + b.genre + ")";
        list_->add(label.c_str(),
                   reinterpret_cast<void*>(static_cast<std::intptr_t>(b.id)));
    }
}

// Возвращает id выбранной группы или -1.
int BandPanel::selectedId() const {
    int idx = list_->value();
    if (idx == 0) return -1;
    auto raw = reinterpret_cast<std::intptr_t>(list_->data(idx));
    return static_cast<int>(raw);
}

// Открывает модальное окно создания группы.
void BandPanel::doAdd() {
    BandEditDialog dlg(app_, nullptr);
    dlg.show();
    while (dlg.shown()) Fl::wait();
    if (dlg.saved()) refresh();
}

// Открывает модальное окно редактирования выбранной группы.
void BandPanel::doEdit() {
    int id = selectedId();
    if (id < 0) {
        fl_alert("Сначала выберите группу в списке");
        return;
    }

    auto band = app_.bands().getById(id);
    if (!band) return;

    BandEditDialog dlg(app_, &*band);
    dlg.show();
    while (dlg.shown()) Fl::wait();
    if (dlg.saved()) refresh();
}

// Удаляет выбранную группу после подтверждения.
// Если на группу ссылаются концерты или музыканты — DbError.
void BandPanel::doDelete() {
    int id = selectedId();
    if (id < 0) {
        fl_alert("Сначала выберите группу в списке");
        return;
    }

    auto band = app_.bands().getById(id);
    if (!band) return;

    int r = fl_choice("Удалить группу \"%s\"?", "Нет", "Да", nullptr,
                      band->name.c_str());
    if (r != 1) return;

    try {
        app_.bands().remove(id);
        refresh();
    } catch (const DbError& e) {
        fl_alert("Не удалось удалить: в группе есть музыканты\n%s", e.what());
    }
}

// Открывает окно с подробной информацией о выбранной группе.
void BandPanel::doDetails() {
    int id = selectedId();
    if (id < 0) {
        fl_alert("Сначала выберите группу в списке");
        return;
    }

    auto band = app_.bands().getById(id);
    if (!band) return;

    BandDetailsDialog dlg(app_, *band);
    dlg.show();
    while (dlg.shown()) Fl::wait();
}

// --- Статические callbacks FLTK ---

void BandPanel::onAdd(Fl_Widget*, void* data) {
    static_cast<BandPanel*>(data)->doAdd();
}

void BandPanel::onEdit(Fl_Widget*, void* data) {
    static_cast<BandPanel*>(data)->doEdit();
}

void BandPanel::onDelete(Fl_Widget*, void* data) {
    static_cast<BandPanel*>(data)->doDelete();
}

void BandPanel::onRefresh(Fl_Widget*, void* data) {
    static_cast<BandPanel*>(data)->refresh();
}

void BandPanel::onDetails(Fl_Widget*, void* data) {
    static_cast<BandPanel*>(data)->doDetails();
}

// ============================================================
// MusicianPanel - панель управления музыкантами
// ============================================================

// Конструктор панели: создаёт список музыкантов и кнопки управления.
// Координаты x, y, w, h передаются из MainWindow и определяют,
// где панель расположена внутри рабочей области.
MusicianPanel::MusicianPanel(int x, int y, int w, int h, App& app)
    : Fl_Group(x, y, w, h),
      app_(app)
{
    // Список музыкантов — верхняя часть, снизу 70 пикселей под кнопки
    list_ = new Fl_Hold_Browser(x + 10, y + 10, w - 20, h - 70);

    // Кнопки управления
    int by = y + h - 50;
    btn_add_     = new Fl_Button(x + 10,  by, 100, 30, "Добавить");
    btn_edit_    = new Fl_Button(x + 120, by, 100, 30, "Изменить");
    btn_delete_  = new Fl_Button(x + 230, by, 100, 30, "Удалить");
    btn_refresh_ = new Fl_Button(x + 340, by, 100, 30, "Обновить");

    btn_add_->callback(onAdd, this);
    btn_edit_->callback(onEdit, this);
    btn_delete_->callback(onDelete, this);
    btn_refresh_->callback(onRefresh, this);

    end();
    refresh();
}

// Перечитывает всех музыкантов из БД и заново заполняет список.
// Формат строки: "Имя — Инструмент (группа)".
// Если инструмент не указан, показывается только имя и группа.
// Имя группы подтягивается через BandService::getById.
void MusicianPanel::refresh() {
    list_->clear();
    auto musicians = app_.musicians().getAll();
    for (auto& m : musicians) {
        // Имя + инструмент
        std::string label = m.name;
        if (!m.phone.empty())
            label += " — " + m.phone;
        if (!m.email.empty())
            label += " — " + m.email;
        if (!m.instrument.empty())
            label += " — " + m.instrument;

        // Имя группы (если группа существует)
        auto band = app_.bands().getById(m.band_id);
        if (band)
            label += " (" + band->name + ")";

        list_->add(label.c_str(),
                   reinterpret_cast<void*>(static_cast<std::intptr_t>(m.id)));
    }
}

// Возвращает id выбранного музыканта или -1.
int MusicianPanel::selectedId() const {
    int idx = list_->value();
    if (idx == 0) return -1;
    auto raw = reinterpret_cast<std::intptr_t>(list_->data(idx));
    return static_cast<int>(raw);
}

// Открывает модальное окно создания музыканта.
void MusicianPanel::doAdd() {
    MusicianEditDialog dlg(app_, nullptr);
    dlg.show();
    while (dlg.shown()) Fl::wait();
    if (dlg.saved()) refresh();
}

// Открывает модальное окно редактирования выбранного музыканта.
void MusicianPanel::doEdit() {
    int id = selectedId();
    if (id < 0) {
        fl_alert("Сначала выберите музыканта в списке");
        return;
    }

    auto musician = app_.musicians().getById(id);
    if (!musician) return;

    MusicianEditDialog dlg(app_, &*musician);
    dlg.show();
    while (dlg.shown()) Fl::wait();
    if (dlg.saved()) refresh();
}

// Удаляет выбранного музыканта после подтверждения.
// Музыкант не имеет входящих ссылок, поэтому удаляется свободно.
void MusicianPanel::doDelete() {
    int id = selectedId();
    if (id < 0) {
        fl_alert("Сначала выберите музыканта в списке");
        return;
    }

    auto musician = app_.musicians().getById(id);
    if (!musician) return;

    int r = fl_choice("Удалить музыканта \"%s\"?", "Нет", "Да", nullptr,
                      musician->name.c_str());
    if (r != 1) return;

    try {
        app_.musicians().remove(id);
        refresh();
    } catch (const DbError& e) {
        fl_alert("Не удалось удалить: %s", e.what());
    }
}

// --- Статические callbacks FLTK ---

void MusicianPanel::onAdd(Fl_Widget*, void* data) {
    static_cast<MusicianPanel*>(data)->doAdd();
}

void MusicianPanel::onEdit(Fl_Widget*, void* data) {
    static_cast<MusicianPanel*>(data)->doEdit();
}

void MusicianPanel::onDelete(Fl_Widget*, void* data) {
    static_cast<MusicianPanel*>(data)->doDelete();
}

void MusicianPanel::onRefresh(Fl_Widget*, void* data) {
    static_cast<MusicianPanel*>(data)->refresh();
}

// ============================================================
// VenuePanel - панель управления площадками
// ============================================================

// Конструктор панели: создаёт список площадок и кнопки управления.
// Координаты x, y, w, h передаются из MainWindow и определяют,
// где панель расположена внутри рабочей области.
VenuePanel::VenuePanel(int x, int y, int w, int h, App& app)
    : Fl_Group(x, y, w, h),
      app_(app)
{
    // Список площадок — верхняя часть, снизу 70 пикселей под кнопки
    list_ = new Fl_Hold_Browser(x + 10, y + 10, w - 20, h - 70);

    // Кнопки управления
    int by = y + h - 50;
    btn_add_     = new Fl_Button(x + 10,  by, 100, 30, "Добавить");
    btn_edit_    = new Fl_Button(x + 120, by, 100, 30, "Изменить");
    btn_delete_  = new Fl_Button(x + 230, by, 100, 30, "Удалить");
    btn_refresh_ = new Fl_Button(x + 340, by, 100, 30, "Обновить");

    btn_add_->callback(onAdd, this);
    btn_edit_->callback(onEdit, this);
    btn_delete_->callback(onDelete, this);
    btn_refresh_->callback(onRefresh, this);

    end();
    refresh();
}

// Перечитывает все площадки из БД и заново заполняет список.
void VenuePanel::refresh() {
    list_->clear();
    auto venues = app_.venues().getAll();
    for (auto& v : venues) {
        std::string label = v.name + " — " + v.address + " — " + v.contact
                          + " (" + std::to_string(v.capacity) + " мест)";
        list_->add(label.c_str(),
                   reinterpret_cast<void*>(static_cast<std::intptr_t>(v.id)));
    }
}

// Возвращает id выбранной площадки или -1.
int VenuePanel::selectedId() const {
    int idx = list_->value();
    if (idx == 0) return -1;
    auto raw = reinterpret_cast<std::intptr_t>(list_->data(idx));
    return static_cast<int>(raw);
}

// Открывает модальное окно создания площадки.
void VenuePanel::doAdd() {
    VenueEditDialog dlg(app_, nullptr);
    dlg.show();
    while (dlg.shown()) Fl::wait();
    if (dlg.saved()) refresh();
}

// Открывает модальное окно редактирования выбранной площадки.
void VenuePanel::doEdit() {
    int id = selectedId();
    if (id < 0) {
        fl_alert("Сначала выберите площадку в списке");
        return;
    }

    auto venue = app_.venues().getById(id);
    if (!venue) return;

    VenueEditDialog dlg(app_, &*venue);
    dlg.show();
    while (dlg.shown()) Fl::wait();
    if (dlg.saved()) refresh();
}

// Удаляет выбранную площадку после подтверждения.
// Если на площадку ссылаются концерты — DbError.
void VenuePanel::doDelete() {
    int id = selectedId();
    if (id < 0) {
        fl_alert("Сначала выберите площадку в списке");
        return;
    }

    auto venue = app_.venues().getById(id);
    if (!venue) return;

    int r = fl_choice("Удалить площадку \"%s\"?", "Нет", "Да", nullptr,
                      venue->name.c_str());
    if (r != 1) return;

    try {
        app_.venues().remove(id);
        refresh();
    } catch (const DbError& e) {
        fl_alert("Не удалось удалить: на площадке есть концерты\n%s", e.what());
    }
}

// --- Статические callbacks FLTK ---

void VenuePanel::onAdd(Fl_Widget*, void* data) {
    static_cast<VenuePanel*>(data)->doAdd();
}

void VenuePanel::onEdit(Fl_Widget*, void* data) {
    static_cast<VenuePanel*>(data)->doEdit();
}

void VenuePanel::onDelete(Fl_Widget*, void* data) {
    static_cast<VenuePanel*>(data)->doDelete();
}

void VenuePanel::onRefresh(Fl_Widget*, void* data) {
    static_cast<VenuePanel*>(data)->refresh();
}

// ============================================================
// ConcertPanel - панель управления концертами
// ============================================================

// Конструктор панели: создаёт список концертов и кнопки управления.
// Дополнительная кнопка "Договор" открывает ContractEditDialog
// для выбранного концерта (создание или редактирование — по наличию).
ConcertPanel::ConcertPanel(int x, int y, int w, int h, App& app)
    : Fl_Group(x, y, w, h),
      app_(app)
{
    list_ = new Fl_Hold_Browser(x + 10, y + 10, w - 20, h - 70);

    int by = y + h - 50;
    btn_add_      = new Fl_Button(x + 10,  by, 100, 30, "Добавить");
    btn_edit_     = new Fl_Button(x + 120, by, 100, 30, "Изменить");
    btn_delete_   = new Fl_Button(x + 230, by, 100, 30, "Удалить");
    btn_refresh_  = new Fl_Button(x + 340, by, 100, 30, "Обновить");
    btn_contract_ = new Fl_Button(x + 450, by, 100, 30, "Договор");

    btn_add_->callback(onAdd, this);
    btn_edit_->callback(onEdit, this);
    btn_delete_->callback(onDelete, this);
    btn_refresh_->callback(onRefresh, this);
    btn_contract_->callback(onContract, this);

    end();
    refresh();
}

// Перечитывает все концерты из БД и заполняет список.
// Имена группы и площадки подтягиваются через getById.
void ConcertPanel::refresh() {
    list_->clear();
    auto concerts = app_.concerts().getAll();
    for (auto& c : concerts) {
        std::string label = c.date;

        auto band = app_.bands().getById(c.band_id);
        if (band) label += " — " + band->name;

        auto venue = app_.venues().getById(c.venue_id);
        if (venue) label += " @ " + venue->name;

        label += " (" + concertStatusToRu(c.status) + ")";

        list_->add(label.c_str(),
                   reinterpret_cast<void*>(static_cast<std::intptr_t>(c.id)));
    }
}

// Возвращает id выбранного концерта или -1.
int ConcertPanel::selectedId() const {
    int idx = list_->value();
    if (idx == 0) return -1;
    auto raw = reinterpret_cast<std::intptr_t>(list_->data(idx));
    return static_cast<int>(raw);
}

// Открывает модальное окно создания концерта.
void ConcertPanel::doAdd() {
    ConcertEditDialog dlg(app_, nullptr);
    dlg.show();
    while (dlg.shown()) Fl::wait();
    if (dlg.saved()) refresh();
}

// Открывает модальное окно редактирования выбранного концерта.
void ConcertPanel::doEdit() {
    int id = selectedId();
    if (id < 0) {
        fl_alert("Сначала выберите концерт в списке");
        return;
    }

    auto concert = app_.concerts().getById(id);
    if (!concert) return;

    ConcertEditDialog dlg(app_, &*concert);
    dlg.show();
    while (dlg.shown()) Fl::wait();
    if (dlg.saved()) refresh();
}

// Удаляет выбранный концерт после подтверждения.
// Если на концерт ссылается договор — DbError.
void ConcertPanel::doDelete() {
    int id = selectedId();
    if (id < 0) {
        fl_alert("Сначала выберите концерт в списке");
        return;
    }

    auto concert = app_.concerts().getById(id);
    if (!concert) return;

    int r = fl_choice("Удалить концерт от %s?", "Нет", "Да", nullptr,
                      concert->date.c_str());
    if (r != 1) return;

    try {
        app_.concerts().remove(id);
        refresh();
    } catch (const DbError& e) {
        fl_alert("Не удалось удалить: у концерта есть контракт\n%s", e.what());
    }
}

// Открывает ContractEditDialog для выбранного концерта.
// Если у концерта уже есть договор — режим редактирования,
// иначе — режим создания с привязкой к этому концерту.
void ConcertPanel::doContract() {
    int id = selectedId();
    if (id < 0) {
        fl_alert("Сначала выберите концерт в списке");
        return;
    }

    auto existing = app_.contracts().findByConcert(id);

    if (existing) {
        fl_alert("У концерта уже есть договор \"%s\".\n"
                 "Чтобы создать новый, сначала удалите существующий "
                 "в разделе \"Договоры\".",
                 existing->number.c_str());
        return;
    } else {
        // Создание нового
        ContractEditDialog dlg(app_, id, nullptr);
        dlg.show();
        while (dlg.shown()) Fl::wait();
    }
    // refresh() не нужен: список концертов не меняется
}

// --- Статические callbacks ---

void ConcertPanel::onAdd(Fl_Widget*, void* data) {
    static_cast<ConcertPanel*>(data)->doAdd();
}

void ConcertPanel::onEdit(Fl_Widget*, void* data) {
    static_cast<ConcertPanel*>(data)->doEdit();
}

void ConcertPanel::onDelete(Fl_Widget*, void* data) {
    static_cast<ConcertPanel*>(data)->doDelete();
}

void ConcertPanel::onRefresh(Fl_Widget*, void* data) {
    static_cast<ConcertPanel*>(data)->refresh();
}

void ConcertPanel::onContract(Fl_Widget*, void* data) {
    static_cast<ConcertPanel*>(data)->doContract();
}

// ============================================================
// ContractPanel - панель управления договорами
// ============================================================

// Конструктор панели: создаёт список договоров и кнопки.
// Кнопки "Добавить" и "Изменить" отсутствуют: договор создаётся
// и редактируется из ConcertPanel (кнопка "Договор" на концерте).
// Здесь только обзор, удаление и обновление.
ContractPanel::ContractPanel(int x, int y, int w, int h, App& app)
    : Fl_Group(x, y, w, h),
      app_(app)
{
    list_ = new Fl_Hold_Browser(x + 10, y + 10, w - 20, h - 70);

    int by = y + h - 50;
    btn_delete_  = new Fl_Button(x + 10, by, 100, 30, "Удалить");
    btn_refresh_ = new Fl_Button(x + 120, by, 100, 30, "Обновить");

    btn_delete_->callback(onDelete, this);
    btn_refresh_->callback(onRefresh, this);

    end();
    refresh();
}

// Перечитывает все договоры из БД и заполняет список.
// Формат строки: "Номер — дата — сумма — статус (концерт: Группа @ Площадка)".
// Имена группы и площадки подтягиваются через findByConcert + getById.
void ContractPanel::refresh() {
    list_->clear();
    auto contracts = app_.contracts().getAll();
    for (auto& ct : contracts) {
        // Основная информация о договоре
        std::string label = ct.number
                          + " — " + ct.date
                          + " — " + std::to_string(ct.amount)
                          + " (" + contractStatusToRu(ct.status) + ")";

        // Информация о связанном концерте (если найден)
        auto concert = app_.concerts().getById(ct.concert_id);
        if (concert) {
            auto band = app_.bands().getById(concert->band_id);
            auto venue = app_.venues().getById(concert->venue_id);

            label += " | ";
            if (band)  label += band->name;
            if (venue) label += " @ " + venue->name;
        }

        list_->add(label.c_str(),
                   reinterpret_cast<void*>(static_cast<std::intptr_t>(ct.id)));
    }
}

// Возвращает id выбранного договора или -1.
int ContractPanel::selectedId() const {
    int idx = list_->value();
    if (idx == 0) return -1;
    auto raw = reinterpret_cast<std::intptr_t>(list_->data(idx));
    return static_cast<int>(raw);
}

// Удаляет выбранный договор после подтверждения.
// У договора нет входящих ссылок, поэтому удаляется свободно.
void ContractPanel::doDelete() {
    int id = selectedId();
    if (id < 0) {
        fl_alert("Сначала выберите договор в списке");
        return;
    }

    auto contract = app_.contracts().getById(id);
    if (!contract) return;

    int r = fl_choice("Удалить договор \"%s\"?", "Нет", "Да", nullptr,
                      contract->number.c_str());
    if (r != 1) return;

    try {
        app_.contracts().remove(id);
        refresh();
    } catch (const DbError& e) {
        fl_alert("Не удалось удалить: %s", e.what());
    }
}

// --- Статические callbacks ---

void ContractPanel::onDelete(Fl_Widget*, void* data) {
    static_cast<ContractPanel*>(data)->doDelete();
}

void ContractPanel::onRefresh(Fl_Widget*, void* data) {
    static_cast<ContractPanel*>(data)->refresh();
}

// ============================================================
// UserPanel - панель управления пользователями
// ============================================================

// Конструктор панели: создаёт список пользователей и кнопки.
// Кнопки "Изменить" нет: пользователя можно удалить и создать заново.
// Доступ к панели обычно ограничен ролью admin (в меню),
// но сама панель проверок роли не делает — это ответственность UI.
UserPanel::UserPanel(int x, int y, int w, int h, App& app)
    : Fl_Group(x, y, w, h),
      app_(app)
{
    list_ = new Fl_Hold_Browser(x + 10, y + 10, w - 20, h - 70);

    int by = y + h - 50;
    btn_add_     = new Fl_Button(x + 10, by, 100, 30, "Добавить");
    btn_delete_  = new Fl_Button(x + 120, by, 100, 30, "Удалить");
    btn_refresh_ = new Fl_Button(x + 230, by, 100, 30, "Обновить");

    btn_add_->callback(onAdd, this);
    btn_delete_->callback(onDelete, this);
    btn_refresh_->callback(onRefresh, this);

    end();
    refresh();
}

// Перечитывает всех пользователей из БД и заполняет список.
void UserPanel::refresh() {
    list_->clear();
    auto users = app_.auth().getAll();
    for (auto& u : users) {
        std::string role = (u.role_id == 1) ? "admin"
                         : (u.role_id == 2) ? "manager"
                                            : "unknown";
        std::string label = u.login + " (" + role + ")";
        list_->add(label.c_str(),
                   reinterpret_cast<void*>(static_cast<std::intptr_t>(u.id)));
    }
}

// Возвращает id выбранного пользователя или -1.
int UserPanel::selectedId() const {
    int idx = list_->value();
    if (idx == 0) return -1;
    auto raw = reinterpret_cast<std::intptr_t>(list_->data(idx));
    return static_cast<int>(raw);
}

// Открывает модальное окно создания пользователя.
void UserPanel::doAdd() {
    if (app_.currentUser().role_id != 1) {
        fl_alert("Доступ только для администратора");
        return;
    }
    UserCreateDialog dlg(app_);
    dlg.show();
    while (dlg.shown()) Fl::wait();
    if (dlg.saved()) refresh();
}

// Удаляет выбранного пользователя после подтверждения.
// Нельзя удалить самого себя (потеряется текущая сессия).
// На пользователя могут ссылаться группы
// тогда SQLite бросит DbError, который мы показываем пользователю.
void UserPanel::doDelete() {
    if (app_.currentUser().role_id != 1) {
        fl_alert("Доступ только для администратора");
        return;
    }
    int id = selectedId();
    if (id < 0) {
        fl_alert("Сначала выберите пользователя в списке");
        return;
    }

    // Защита от самоудаления
    if (id == app_.currentUser().id) {
        fl_alert("Нельзя удалить текущего пользователя");
        return;
    }

    // Находим логин для сообщения
    auto users = app_.auth().getAll();
    std::string login;
    for (auto& u : users) {
        if (u.id == id) { login = u.login; break; }
    }

    int r = fl_choice("Удалить пользователя \"%s\"?", "Нет", "Да", nullptr,
                      login.c_str());
    if (r != 1) return;

    try {
        app_.auth().remove(id);
        refresh();
    } catch (const DbError& e) {
        fl_alert("Не удалось удалить: у пользователя есть группы\n%s", e.what());
    }
}

// --- Статические callbacks ---

void UserPanel::onAdd(Fl_Widget*, void* data) {
    static_cast<UserPanel*>(data)->doAdd();
}

void UserPanel::onDelete(Fl_Widget*, void* data) {
    static_cast<UserPanel*>(data)->doDelete();
}

void UserPanel::onRefresh(Fl_Widget*, void* data) {
    static_cast<UserPanel*>(data)->refresh();
}

// ============================================================
// ReportPanel - панель отчётов
// ============================================================

// Конструктор панели: создаёт поля периода, кнопку и область вывода.
// Период по умолчанию — с начала текущего месяца по сегодняшний день.
ReportPanel::ReportPanel(int x, int y, int w, int h, App& app)
    : Fl_Group(x, y, w, h),
      app_(app)
{
    // Заголовок и поля периода
    auto* label = new Fl_Box(x + 10, y + 10, w - 20, 22, "Отчёт за период:");
    label->labelfont(FL_BOLD);
    label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    input_from_ = new Fl_Input(x + 80, y + 40, 130, 25, "от:");
    input_from_->align(FL_ALIGN_LEFT);
    input_from_->value(today().c_str());   // по умолчанию — сегодня

    input_to_ = new Fl_Input(x + 280, y + 40, 130, 25, "до:");
    input_to_->align(FL_ALIGN_LEFT);
    input_to_->value(today().c_str());

    btn_build_ = new Fl_Button(x + 440, y + 40, 120, 25, "Построить");
    btn_build_->callback(onBuild, this);

    // Область вывода — на всю оставшуюся высоту
    output_ = new Fl_Hold_Browser(x + 10, y + 80, w - 20, h - 100);

    end();
}

// Собирает отчёт за выбранный период и заполняет output_.
// Период берётся из input_from_ и input_to_ (формат YYYY-MM-DD).
// Все данные читаются через сервисы, ничего не изменяется.
void ReportPanel::buildReport() {
    output_->clear();

    std::string from = input_from_->value() ? input_from_->value() : "";
    std::string to   = input_to_->value()   ? input_to_->value()   : "";

    if (from.empty() || to.empty()) {
        output_->add("Задайте период в формате YYYY-MM-DD");
        return;
    } else if (!isValidDate(from) || !isValidDate(to)) {
        output_->add("Ошибка: дата должна быть в формате YYYY-MM-DD");
        return;
    }

    if (from > to) {
        output_->add("Ошибка: дата \"от\" позже даты \"до\"");
        return;
    }

    // Получаем все концерты за период
    auto concerts = app_.concerts().findByDateRange(from, to);

    // --- Сводка ---
    output_->add("=== Сводка по концертам ===");
    output_->add(("Период: " + from + " — " + to).c_str());
    output_->add("");

    double total_fee = 0.0;
    double total_expenses = 0.0;
    for (auto& c : concerts) {
        total_fee += c.fee;
        total_expenses += c.expenses;
    }

    char buf[256];
    std::snprintf(buf, sizeof(buf), "Всего концертов: %d", (int)concerts.size());
    output_->add(buf);

    std::snprintf(buf, sizeof(buf), "Общий гонорар: %.2f", total_fee);
    output_->add(buf);

    std::snprintf(buf, sizeof(buf), "Общие расходы: %.2f", total_expenses);
    output_->add(buf);

    std::snprintf(buf, sizeof(buf), "Прибыль: %.2f", total_fee - total_expenses);
    output_->add(buf);

    output_->add("");

    // --- Разбивка по группам ---
    output_->add("=== Разбивка по группам ===");

    // Для каждой группы считаем число концертов и сумму гонораров
    auto bands = app_.bands().getAll();
    for (auto& b : bands) {
        int count = 0;
        double sum_fee = 0.0;
        for (auto& c : concerts) {
            if (c.band_id == b.id) {
                ++count;
                sum_fee += c.fee;
            }
        }
        if (count > 0) {
            std::snprintf(buf, sizeof(buf),
                "%s — концертов: %d, гонорар: %.2f",
                b.name.c_str(), count, sum_fee);
            output_->add(buf);
        }
    }

    output_->add("");

    // --- Неоплаченные договоры ---
    output_->add("=== Неоплаченные договоры ===");

    int unpaid = 0;
    auto contracts = app_.contracts().getAll();
    for (auto& ct : contracts) {
        std::string ru_status = contractStatusToRu(ct.status);
        if (ct.status == "draft" || ct.status == "signed") {
            std::snprintf(buf, sizeof(buf),
                "%s — %.2f (%s)",
                ct.number.c_str(), ct.amount, ru_status.c_str());
            output_->add(buf);
            ++unpaid;
        }
    }
    if (unpaid == 0) {
        output_->add("(нет)");
    }
}

// Статический callback кнопки "Построить".
void ReportPanel::onBuild(Fl_Widget*, void* data) {
    static_cast<ReportPanel*>(data)->buildReport();
}
// Статический метод для валидации даты
bool ReportPanel::isValidDate(const std::string& s) {
    if (s.size() != 10)
        return false;
    if (s[4] != '-' || s[7] != '-')
        return false;
    for (size_t i = 0; i < s.size(); ++i) {
        if (i == 4 || i == 7)
            continue;
        if (!std::isdigit(static_cast<unsigned char>(s[i])))
            return false;
    }
    return true;
}
