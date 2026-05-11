#include "../include/DB-wrapper.h"
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <thread>


// Пока не используется
size_t DB_wrapper::totalDocuments() {
    try {
        mongocxx::client conn{mongocxx::uri{"mongodb://localhost:27017"}};
        return conn["sdr_data"]["signals"].count_documents({});
    } catch (const std::exception &e) {
        std::cerr << "[DB] Ошибка totalDocuments: " << e.what() << std::endl;
        return 0;
    }
}

// Сервер запускается вручную
bool DB_wrapper::startSever() {
    std::cout << "[DB] Запуск сервера..." << std::endl;
    if (system("sudo systemctl start mongod") != 0) return false;
    for (int i = 0; i < 10; i++) {
        if (isServerRunning()) {
            std::cout << "[DB] Сервер запущен." << std::endl;
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return false;
}

// Сервер останавлтвается вручную
bool DB_wrapper::stopServer() {
    return system("sudo systemctl stop mongod") == 0;
}


// Пока не используется
bool DB_wrapper::isServerRunning() {
    return system("sudo systemctl is-active --quiet mongod") == 0;
}

// Конструктор для видимости, что объект создаётся
DB_wrapper::DB_wrapper() {
    std::cout << "[DB] Конструктор" << std::endl;
}


/////////////////        MONGOSH           ////////////////////
// 1. Запуск монго. По итогу не нужен
bool DB_wrapper::start_mongosh() {
    if (system("mongosh") != 0) return false;

    std::cout << "[DB] mongosh подключен" << std::endl;
    return true;
}


// 2. Вывод состояния БД sdr_data. Все Бд не вывожу, так как для проекта незачем, но можно и сделать
bool DB_wrapper::status_mongosh() {
    system("mongosh --quiet --eval \""
           "var db = db.getSiblingDB('sdr_data');"
           "db.signals.find({}, {iq_data: 0}).sort({_id: -1}).limit(6).forEach(printjson)"
           "\"");
    //system("mongosh");
    //system("show dbs");
    //system("use sdr_data");
    return true;
}


//////////////////  name for data ////////////////////////

// 1.
void
DB_wrapper::addBlock(const std::vector<uint8_t> &data, int center_freq, int sample_rate, const std::string &filename) {
    // Сохраняем параметры первого блока
    if (buffer_.empty()) {              // если это первый блок, то запоминаем параметры. У остальных они не меняются
        buffer_center_freq_ = center_freq;
        buffer_sample_rate_ = sample_rate;
        buffer_filename_ = filename;
    }

    buffer_.push_back(data); // копия вектора в буффер

    // Если накопили лимит — сохраняем
    if (buffer_.size() >= max_blocks_) {
        saveData();
    }
}

// 2.
void DB_wrapper::saveData() {
    if (buffer_.empty()) return;// если пусто, сохранять нечего

    try {
        mongocxx::client conn{mongocxx::uri{"mongodb://localhost:27017"}};
        auto coll = conn["sdr_data"]["signals"];

        // 1. Суммируем размеры
        size_t total_size = 0;
        for (size_t i = 0; i < buffer_.size(); i++) {
            total_size += buffer_[i].size();
        } // проход по всем блокам и суммируем их размеры. 60 * 262144 байт

        // 2. Суммируем блоки
        std::vector<uint8_t> merged; // общий массив для всех
        merged.reserve(total_size); // память под всё
        for (size_t i = 0; i < buffer_.size(); i++) {
            merged.insert(merged.end(), buffer_[i].begin(), buffer_[i].end());
        } // добавляем очередной блок в конец. Всего их 60 штук

        bsoncxx::types::b_binary bin{
                bsoncxx::binary_sub_type::k_binary,
                static_cast<uint32_t>(merged.size()),
                merged.data()
        };

        // BSON объект из данных.
        auto doc = bsoncxx::builder::stream::document{}
                << "timestamp" << bsoncxx::types::b_date(std::chrono::system_clock::now())
                << "center_freq" << buffer_center_freq_
                << "sample_rate" << buffer_sample_rate_
                << "blocks_count" << static_cast<int32_t>(buffer_.size()) // 60
                << "total_samples" << static_cast<int32_t>(total_size / 2) // 7864320 - всего сэмплов
                << "filename" << buffer_filename_
                << "iq_data" << bin
                << bsoncxx::builder::stream::finalize;

        coll.insert_one(doc.view());
        blocks_saved_ += buffer_.size();
        std::cout << "[DB] Сохранено " << buffer_.size() << " блоков (" << total_size / 2 << " сэмплов)"
                  << std::endl;

        buffer_.clear(); // очищаем для следующей пачки

    } catch (const std::exception &e) {
        std::cerr << "[DB] Ошибка при flush: " << e.what() << std::endl;
    }
}
// try-catch на всякий случай, если код в true упадёт.


//////// НЕ АКТУально /////
// Не актуально. saveData новый вариант
void DB_wrapper::saveIQData(const std::vector<uint8_t> &data, int center_freq, int sample_rate,
                            const std::string &filename) {
    try {
        mongocxx::client conn{mongocxx::uri{"mongodb://localhost:27017"}};
        auto coll = conn["sdr_data"]["signals"];

        using bsoncxx::builder::stream::document;
        using bsoncxx::builder::stream::finalize;
        using bsoncxx::types::b_binary;
        using bsoncxx::binary_sub_type;

        bsoncxx::types::b_binary bin{
                bsoncxx::binary_sub_type::k_binary, // тип: обычные байты
                static_cast<uint32_t>(data.size()), // размер в байтах
                data.data()                     // указатель на данные
        };

        auto doc = document{}
                << "timestamp" << bsoncxx::types::b_date(std::chrono::system_clock::now())
                << "center_freq" << center_freq
                << "sample_rate" << sample_rate
                << "sample_count" << static_cast<int64_t>(data.size() / 2)
                << "filename" << filename
                << "iq_data" << bin
                //<< "filename" << "FM88400000_2048000_*.bin"  // ссылка на файл
                << finalize;

        coll.insert_one(doc.view());
        std::cout << "[DB] Сохранено: " << data.size() / 2 << " сэмплов на " << center_freq << " Гц" << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "[DB] Ошибка сохранения: " << e.what() << std::endl;
    }
}
