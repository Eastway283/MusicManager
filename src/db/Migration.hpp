#pragma once

class Database;

// Создает все таблицы, если их нет. Вызывается один раз при старте
void runMigration(Database& db);
