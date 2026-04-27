#include <iostream>      // Для cout, cin (консоль)
#include <thread>        // Для std::thread (потоки)
#include <mutex>         // Для std::mutex (защита данных)
#include "../header/stream.h"

void BackgroundWorker::start() {
    //std::lock_guard<std::mutex> lock(mtx); // Вешаем замок
    if (running) return; // уже работает - выходим
    running = true;

    worker = std::thread(&BackgroundWorker::work, this); // Создаём поток -> вызов work.cpp -> readSignal

    std::cout << "[WORKER] Запущен.\n";
}
