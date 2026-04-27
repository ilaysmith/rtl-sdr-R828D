#include <iostream>      // Для cout, cin (консоль)
#include <thread>        // Для std::thread (потоки)
#include <atomic>        // Для атомарных операций (не использован, но полезен)
#include <mutex>         // Для std::mutex (защита данных)
#include <condition_variable> // Для условных переменных (не использован)
#include <chrono>        // Для sleep_for (задержка)
#include <string>        // Для std::string
#include "../header/stream.h"
#include "../../sdr-rtl/header/RTL_SDR.h"


void BackgroundWorker::set() {

    device.setParameters();

}