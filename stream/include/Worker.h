#ifndef SDRTEST_WORKER_H
#define SDRTEST_WORKER_H

#include <iostream>      // Для cout, cin (консоль)
#include <thread>        // Для std::thread (потоки)
#include <atomic>        // Для атомарных операций (не использован, но полезен)
#include <mutex>         // Для std::mutex (защита данных)
#include <condition_variable> // Для условных переменных (не использован)
#include <chrono>        // Для sleep_for (задержка)
#include <string>        // Для std::string
#include "../../rtl-sdr-wrapper/include/rtl-sdr-wrapper.h"
#include "../../DB-wrapper/include/DB-wrapper.h"

class Worker {
public:

    Worker() : running(false), counter(0) {}

    ~Worker() {
        stop();
        if (worker.joinable()) worker.join();
    }

    std::thread worker; // объект потока

    std::atomic<bool> running; // флаг обозначения записи

    rtlsdr_wrapper device; // объект rtl-sdr

    Recorder recorder; // объект записи данных в файл

    DB_wrapper db_wrapper; // обект БД


    uint64_t counter; // обычный счётчик

    // Методы управления чтением данных с rtl-sdr
    void start();

    void init();

    void set();

    void work();

    void stop();

    void exit();

    void status();

    //mutable std::mutex mtx; // mutex - замок для защиты данных
};

#endif //SDRTEST_WORKER_H
