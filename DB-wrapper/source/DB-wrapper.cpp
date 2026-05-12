#include "../include/DB-wrapper.h"
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>
#include <chrono>
#include <thread>

// Конструктор для видимости, что объект создаётся
DB_wrapper::DB_wrapper() {
    std::cout << "[DB] Конструктор" << std::endl;
}

//////////////////  Save data ////////////////////////

// 1. Сохранение данных
void DB_wrapper::saveData(int center_freq, int sample_rate,
                          const std::string &filename,
                          int blocks_count, int total_samples) {
    try {
        mongocxx::client conn{mongocxx::uri{uri_}};
        auto coll = conn[db_name_][collection_name_];

        auto doc = bsoncxx::builder::stream::document{}
                << "timestamp" << bsoncxx::types::b_date(std::chrono::system_clock::now())
                << "center_freq" << center_freq
                << "sample_rate" << sample_rate
                << "blocks_count" << blocks_count
                << "total_samples" << total_samples
                << "filename" << filename
                << bsoncxx::builder::stream::finalize;

        coll.insert_one(doc.view());
        std::cout << "[DB] Метаданные сохранены: " << filename
                  << " (" << blocks_count << " блоков)" << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "[DB] Ошибка: " << e.what() << std::endl;
    }
}

// try-catch на всякий случай, если код в true упадёт.


// 2. Вывод последних пяти документов
void DB_wrapper::printLastRecords(int count) {
    try {
        mongocxx::client conn{mongocxx::uri{uri_}}; // временное подключение к серверу
        auto coll = conn[db_name_][collection_name_]; // выбирает конкретный документ и коллекцию

        // Сортировка по убыванию _id (новые сначала), ограничение
        auto opts = mongocxx::options::find{}; // объект для запросов
        opts.sort(bsoncxx::builder::stream::document{}  // задаёт сортировку
                          << "_id" << -1                        // сортировка по "id" по убыванию "-1"
                          << bsoncxx::builder::stream::finalize);
        opts.limit(count);                                  // ограничивает количество результатов - 5

        auto cursor = coll.find({}, opts);  // ищет все документы (фильтр пустой) с заданными настройками opts
        // возвращает курсор - объект, который подгружает документы по одному

        // Вывод результатов
        int printed = 0;
        for (auto &doc: cursor) {           // на каждой итерации.    doc - bson документ
            std::cout << bsoncxx::to_json(doc) << std::endl;        // преобразует bson в читаему json
            printed++;
        }

        if (printed == 0) {
            std::cout << "[DB] Коллекция пуста." << std::endl;
        }

    } catch (const std::exception &e) {
        std::cerr << "[DB] Ошибка при чтении: " << e.what() << std::endl;
    }
}

// 3.
void DB_wrapper::addBlock(int center_freq, int sample_rate, const std::string &filename) {
    // Если файл сменился — сохраняем статистику старого
    if (!current_filename_.empty() && filename != current_filename_) {
        saveData(center_freq_, sample_rate_, current_filename_, blocks_count_, blocks_count_ * 262144 / 2);
        blocks_count_ = 0;
    }
    // Запоминаем текущий файл
    current_filename_ = filename;
    center_freq_ = center_freq;
    sample_rate_ = sample_rate;
    blocks_count_++;
}