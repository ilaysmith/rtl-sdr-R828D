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

class Worker {
public:
    //mutable std::mutex mtx; // mutex - замок для защиты данных
    std::thread worker; // объект потока, но не представляет никакой поток

    std::atomic<bool> running;

    uint64_t counter;

    rtlsdr_wrapper device;

    Recorder recorder;

    Worker() : running(false), counter(0) {}

    void start();

    void init();

    void set();

    void work();

    void stop();

    void exit();

    ~Worker() {
        stop();
        if (worker.joinable()) worker.join();
    }
};

#endif //SDRTEST_WORKER_H
