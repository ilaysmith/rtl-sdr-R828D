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

void DB_wrapper::saveIQData(const std::vector<uint8_t> &data, int center_freq, int sample_rate) {
    try {
        mongocxx::client conn{mongocxx::uri{"mongodb://localhost:27017"}};
        auto coll = conn["sdr_data"]["signals"];

        using bsoncxx::builder::stream::document;
        using bsoncxx::builder::stream::finalize;
        using bsoncxx::types::b_binary;
        using bsoncxx::binary_sub_type;

        bsoncxx::types::b_binary bin{
                bsoncxx::binary_sub_type::k_binary,
                static_cast<uint32_t>(data.size()),
                data.data()
        };

        auto doc = document{}
                << "timestamp" << bsoncxx::types::b_date(std::chrono::system_clock::now())
                << "center_freq" << center_freq
                << "sample_rate" << sample_rate
                << "sample_count" << static_cast<int64_t>(data.size() / 2)
                << "iq_data" << bin
                //<< "filename" << "FM88400000_2048000_*.bin"  // ссылка на файл
                << finalize;

        coll.insert_one(doc.view());
        std::cout << "[DB] Сохранено: " << data.size() / 2 << " сэмплов на " << center_freq << " Гц" << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "[DB] Ошибка сохранения: " << e.what() << std::endl;
    }
}

size_t DB_wrapper::totalDocuments() {
    try {
        mongocxx::client conn{mongocxx::uri{"mongodb://localhost:27017"}};
        return conn["sdr_data"]["signals"].count_documents({});
    } catch (const std::exception &e) {
        std::cerr << "[DB] Ошибка totalDocuments: " << e.what() << std::endl;
        return 0;
    }
}

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

bool DB_wrapper::stopServer() {
    return system("sudo systemctl stop mongod") == 0;
}

bool DB_wrapper::isServerRunning() {
    return system("sudo systemctl is-active --quiet mongod") == 0;
}

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
           "db.signals.find({}, {iq_data: 0}).limit(5).forEach(printjson)"
           "\"");
    //system("mongosh");
    //system("show dbs");
    //system("use sdr_data");
    return true;
}

