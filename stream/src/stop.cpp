#include <iostream>      // Для cout, cin (консоль)
#include <thread>        // Для std::thread (потоки)
#include <atomic>        // Для атомарных операций (не использован, но полезен)
#include <mutex>         // Для std::mutex (защита данных)
#include <condition_variable> // Для условных переменных (не использован)
#include <chrono>        // Для sleep_for (задержка)
#include <string>        // Для std::string
#include "../header/stream.h"


void BackgroundWorker::stop() {
    {
        running = false; // не требует mutex. для остановки work()
        //std::lock_guard<std::mutex> lock(mtx);
        //if (!running) return;

        if (worker.joinable()) worker.join(); // Ждём реального завершения

        //running = false; // Просто ставим флаг, который будет видно в start()
        std::cout << "[WORKER] Остановлен. \n";
    }
}