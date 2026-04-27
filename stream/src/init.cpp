#include <iostream>      // Для cout, cin (консоль)
#include <mutex>         // Для std::mutex (защита данных)
#include "../header/stream.h"

void BackgroundWorker::init() {
    //std::lock_guard<std::mutex> lock(mtx); // Вешаем замок
    if (running) return; // уже работает - выходим

    device.connectionSDR(); // инициализируем rtl-sdr

    std::cout << "[WORKER] инициализирован.\n";
}
