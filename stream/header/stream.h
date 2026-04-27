#ifndef SDRTEST_STREAM_H
#define SDRTEST_STREAM_H

#include <iostream>      // Для cout, cin (консоль)
#include <thread>        // Для std::thread (потоки)
#include <atomic>        // Для атомарных операций (не использован, но полезен)
#include <mutex>         // Для std::mutex (защита данных)
#include <condition_variable> // Для условных переменных (не использован)
#include <chrono>        // Для sleep_for (задержка)
#include <string>        // Для std::string
#include "../../sdr-rtl/header/RTL_SDR.h"

class BackgroundWorker {
public:
    mutable std::mutex mtx; // mutex - замок для защиты данных
    std::thread worker;

    std::atomic<bool> running;

    uint64_t counter;

    RTL_SDR device;

    BackgroundWorker() : running(false), counter(0) {}

    void start();

    void init();

    void set();

    void work();

    void stop();

    void exit();

    ~BackgroundWorker() {
        stop();
        if (worker.joinable()) worker.join();
    }
};

#endif //SDRTEST_STREAM_H
