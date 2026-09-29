#pragma once

#include "ui/Windows.hpp"   // для PanelType
#include <string>

// Данный заголовочный файл содержит объявления всех панелей интерфейса.
// Панели отображаются в рабочей области MainWindow и переключаются
// пунктами меню. Каждая панель следует единому паттерну:
// список записей + кнопки управления (создать, изменить, удалить, обновить).

class Fl_Hold_Browser;
class Fl_Button;
class Fl_Input;
class App;

// ============================================================
// UserPanel - панель управления пользователями (только для админа)
// ============================================================

// Панель управления пользователями системы.
// Доступна только пользователям с ролью "admin".
// Позволяет создавать, редактировать и удалять учётные записи,
// а также менять роли и сбрасывать пароли.
class UserPanel : public Fl_Group {
    public:
        UserPanel(int x, int y, int w, int h, App& app);

        // Перечитывает список пользователей из БД и обновляет отображение
        void refresh();

    private:
        App& app_;

        // Виджеты
        Fl_Hold_Browser* list_        = nullptr;   // список пользователей
        Fl_Button*       btn_add_     = nullptr;   // кнопка "Добавить"
        Fl_Button*       btn_edit_    = nullptr;   // кнопка "Изменить"
        Fl_Button*       btn_delete_  = nullptr;   // кнопка "Удалить"
        Fl_Button*       btn_refresh_ = nullptr;   // кнопка "Обновить"

        // Возвращает id выбранного пользователя или -1, если ничего не выбрано
        int selectedId() const;

        // Нестатические реализации обработчиков кнопок
        void doAdd();
        void doEdit();
        void doDelete();

        // Статические callbacks FLTK. Получают this через data
        // и вызывают соответствующий do*-метод.
        static void onAdd(Fl_Widget*, void*);
        static void onEdit(Fl_Widget*, void*);
        static void onDelete(Fl_Widget*, void*);
        static void onRefresh(Fl_Widget*, void*);
};

// ============================================================
// BandPanel - панель управления группами
// ============================================================

// Панель управления группами (таблица bands).
// Список групп, кнопки создания/редактирования/удаления.
// Создание и редактирование выполняются через форму внутри панели.
class BandPanel : public Fl_Group {
    public:
        BandPanel(int x, int y, int w, int h, App& app);

        // Перечитывает список групп из БД и обновляет отображение
        void refresh();

    private:
        App& app_;

        // Виджеты
        Fl_Hold_Browser* list_        = nullptr;   // список групп
        Fl_Button*       btn_add_     = nullptr;   // кнопка "Добавить"
        Fl_Button*       btn_edit_    = nullptr;   // кнопка "Изменить"
        Fl_Button*       btn_delete_  = nullptr;   // кнопка "Удалить"
        Fl_Button*       btn_refresh_ = nullptr;   // кнопка "Обновить"
        Fl_Button*       btn_details_ = nullptr;   // кнопка "Подробнее"

        // Возвращает id выбранной группы или -1, если ничего не выбрано
        int selectedId() const;

        // Нестатические реализации обработчиков кнопок
        void doAdd();
        void doEdit();
        void doDelete();
        void doDetails();

        // Статические callbacks FLTK
        static void onAdd(Fl_Widget*, void*);
        static void onEdit(Fl_Widget*, void*);
        static void onDelete(Fl_Widget*, void*);
        static void onRefresh(Fl_Widget*, void*);
        static void onDetails(Fl_Widget*, void*);
};

// ============================================================
// MusicianPanel - панель управления музыкантами
// ============================================================

// Панель управления музыкантами (таблица musicians).
// Список музыкантов, кнопки создания/редактирования/удаления.
// Каждый музыкант привязан к группе (band_id).
class MusicianPanel : public Fl_Group {
    public:
        MusicianPanel(int x, int y, int w, int h, App& app);

        // Перечитывает список музыкантов из БД и обновляет отображение
        void refresh();

    private:
        App& app_;

        // Виджеты
        Fl_Hold_Browser* list_        = nullptr;   // список музыкантов
        Fl_Button*       btn_add_     = nullptr;   // кнопка "Добавить"
        Fl_Button*       btn_edit_    = nullptr;   // кнопка "Изменить"
        Fl_Button*       btn_delete_  = nullptr;   // кнопка "Удалить"
        Fl_Button*       btn_refresh_ = nullptr;   // кнопка "Обновить"

        // Возвращает id выбранного музыканта или -1, если ничего не выбрано
        int selectedId() const;

        // Нестатические реализации обработчиков кнопок
        void doAdd();
        void doEdit();
        void doDelete();

        // Статические callbacks FLTK
        static void onAdd(Fl_Widget*, void*);
        static void onEdit(Fl_Widget*, void*);
        static void onDelete(Fl_Widget*, void*);
        static void onRefresh(Fl_Widget*, void*);
};

// ============================================================
// VenuePanel - панель управления площадками
// ============================================================

// Панель управления площадками (таблица venues).
// Список площадок, кнопки создания/редактирования/удаления.
// Площадка - справочник, используется концертами.
class VenuePanel : public Fl_Group {
    public:
        VenuePanel(int x, int y, int w, int h, App& app);

        // Перечитывает список площадок из БД и обновляет отображение
        void refresh();

    private:
        App& app_;

        // Виджеты
        Fl_Hold_Browser* list_        = nullptr;   // список площадок
        Fl_Button*       btn_add_     = nullptr;   // кнопка "Добавить"
        Fl_Button*       btn_edit_    = nullptr;   // кнопка "Изменить"
        Fl_Button*       btn_delete_  = nullptr;   // кнопка "Удалить"
        Fl_Button*       btn_refresh_ = nullptr;   // кнопка "Обновить"

        // Возвращает id выбранной площадки или -1, если ничего не выбрано
        int selectedId() const;

        // Нестатические реализации обработчиков кнопок
        void doAdd();
        void doEdit();
        void doDelete();

        // Статические callbacks FLTK
        static void onAdd(Fl_Widget*, void*);
        static void onEdit(Fl_Widget*, void*);
        static void onDelete(Fl_Widget*, void*);
        static void onRefresh(Fl_Widget*, void*);
};

// ============================================================
// ConcertPanel - панель управления концертами
// ============================================================

// Панель управления концертами (таблица concerts).
// Список концертов с датами, группами и площадками.
// Фильтры по дате, группе, статусу; смена статуса концерта.
class ConcertPanel : public Fl_Group {
    public:
        ConcertPanel(int x, int y, int w, int h, App& app);

        // Перечитывает список концертов из БД и обновляет отображение
        void refresh();

    private:
        App& app_;

        // Виджеты
        Fl_Hold_Browser* list_         = nullptr;   // список концертов
        Fl_Button*       btn_add_      = nullptr;   // кнопка "Добавить"
        Fl_Button*       btn_edit_     = nullptr;   // кнопка "Изменить"
        Fl_Button*       btn_delete_   = nullptr;   // кнопка "Удалить"
        Fl_Button*       btn_refresh_  = nullptr;   // кнопка "Обновить"
        Fl_Button*       btn_contract_ = nullptr;   // кнопка "Договор"

        // Возвращает id выбранного концерта или -1, если ничего не выбрано
        int selectedId() const;

        // Нестатические реализации обработчиков кнопок
        void doAdd();
        void doEdit();
        void doDelete();
        void doContract();

        // Статические callbacks FLTK
        static void onAdd(Fl_Widget*, void*);
        static void onEdit(Fl_Widget*, void*);
        static void onDelete(Fl_Widget*, void*);
        static void onRefresh(Fl_Widget*, void*);
        static void onContract(Fl_Widget*, void*);
};

// ============================================================
// ContractPanel - панель управления договорами
// ============================================================

// Панель управления договорами (таблица contracts).
// Список договоров с номерами, датами и статусами.
// Каждый договор привязан к концерту (concert_id).
class ContractPanel : public Fl_Group {
    public:
        ContractPanel(int x, int y, int w, int h, App& app);

        // Перечитывает список договоров из БД и обновляет отображение
        void refresh();

    private:
        App& app_;

        // Виджеты
        Fl_Hold_Browser* list_        = nullptr;   // список договоров
        Fl_Button*       btn_add_     = nullptr;   // кнопка "Добавить"
        Fl_Button*       btn_edit_    = nullptr;   // кнопка "Изменить"
        Fl_Button*       btn_delete_  = nullptr;   // кнопка "Удалить"
        Fl_Button*       btn_refresh_ = nullptr;   // кнопка "Обновить"

        // Возвращает id выбранного договора или -1, если ничего не выбрано
        int selectedId() const;

        // Нестатические реализации обработчиков кнопок
        void doAdd();
        void doEdit();
        void doDelete();

        // Статические callbacks FLTK
        static void onAdd(Fl_Widget*, void*);
        static void onEdit(Fl_Widget*, void*);
        static void onDelete(Fl_Widget*, void*);
        static void onRefresh(Fl_Widget*, void*);
};

// ============================================================
// ReportPanel - панель отчётов
// ============================================================

// Панель отчётов: выбор периода, кнопка "Построить",
// вывод сводки в браузере. Ничего не редактирует,
// только читает данные через сервисы.
class ReportPanel : public Fl_Group {
    public:
        ReportPanel(int x, int y, int w, int h, App& app);

    private:
        App& app_;

        Fl_Input*        input_from_  = nullptr;   // дата "от"  "YYYY-MM-DD"
        Fl_Input*        input_to_    = nullptr;   // дата "до"  "YYYY-MM-DD"
        Fl_Button*       btn_build_   = nullptr;   // кнопка "Построить"
        Fl_Hold_Browser* output_      = nullptr;   // вывод отчёта

        // Собирает отчёт за период и заполняет output_
        void buildReport();

        static void onBuild(Fl_Widget*, void*);
        static bool isValidDate(const std::string& s);
};
