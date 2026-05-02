#include <iostream>
#include "../include/Worker.h"
#include <mutex>         // Для std::mutex (защита данных)
#include <atomic>        // Для атомарных операций
#include <thread>        // Для std::thread (потоки)
#include "../../Recorder/include/Recorder.h"

// 1. Метод выхода
void Worker::exit() {

    device.stopRecording();
    std::cout << "[WORKER] Устройство отключено. \n";
}

// 2. Метод инициализации  __________________________  как будто уже не участвует
void Worker::init() {
    //std::lock_guard<std::mutex> lock(mtx); // Вешаем замок
    if (running) return; // уже работает - выходим

    // Device
    device.connectionSDR(); // инициализируем rtl-sdr
    std::cout << "Успешно инициализирован [DEVICE]" << std::endl;

    // Recorder. А надо ли, если у меня в start привязка к инициализации __________________________
    // Инициализируем рекордер только при первом запуске
    static bool recorder_initialized = false;
    if (!recorder_initialized) {
        recorder.configure(device.center_freq, device.sample_rate, "../records", 100);
        recorder_initialized = true;
        std::cout << "Успешно инициализирован [RECORDER]" << std::endl;
    }
    running = true;
    std::cout << "Пока что не инициализирован [DB]" << std::endl << std::endl;

    // DB

    //std::cout << "[WORKER] инициализирован.\n";
}

// 3. Метод установки несущей и ЧД. Пока что захардкожен _____________ в теории можно добавить, чтобы можно было менять ЧД и ЦД
void Worker::set() {

    // Сначала инициализируем устройство
    if (!device.dev) {                    // если устройство ещё не открыто
        device.connectionSDR();           // открываем
    }

    device.setParameters();

}

// 4. Метод запуска
void Worker::start() {
    //std::lock_guard<std::mutex> lock(mtx); // Вешаем замок

    // Инициализация
    init();
    // Установка параметров по умолчанию
    set();

    worker = std::thread(&Worker::work, this); // Создаём поток -> вызов work.cpp -> readSignal
    // в конструктор передаём указатель на метод класса, this - указатель на текущий объект класса, который вызвал start()

    std::cout << "[WORKER] Запущен.\n" << std::endl;
}

// 5. Метод остановки записи сигнала
void Worker::stop() {
    {
        running = false; // не требует mutex. для остановки work()
        //std::lock_guard<std::mutex> lock(mtx);
        //if (!running) return;

        if (worker.joinable()) worker.join(); // Проверяем, что поток существует и ждём реального завершения

        //running = false; // Просто ставим флаг, который будет видно в start()
        recorder.close();
        std::cout << "[WORKER] Остановлен. \n";
    }
}

// Управление потоками
// joinable() — проверяет, возможно ли присоединение связанного потока.
// join() — блокируется до завершения соответствующего потока.

// 6. Метод записи сигнала
void Worker::work() {

    std::cout << "[WORKER] Начало асинхронной записи..." << std::endl;

    device.startRecordingAsync(recorder);

    // Ждём сигнал остановки
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Останавливаем
    device.stopRecordingAsync();
    //device.stopRecording();
    //recorder.close();

    std::cout << "[WORKER] Запись остановлена." << std::endl;
    std::cout << "[WORKER] Всего блоков: " << recorder.totalBlocksWritten() << std::endl;
    std::cout << "[WORKER] Последний файл: " << recorder.currentFilename() << std::endl;
}

