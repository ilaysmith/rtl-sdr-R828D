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
    recorder.close();
    std::cout << "[RECORDER] Запись остановлена. \n";
    std::cout << "[DB] Сервер отключен. \n";
}

// 2. Метод инициализации  __________________________  как будто уже не участвует
void Worker::init() {
    //std::lock_guard<std::mutex> lock(mtx); // Вешаем замок
    if (running) return; // уже работает - выходим

    // Device
    device.connectionSDR(); // инициализируем rtl-sdr
    std::cout << "Успешно инициализирован [DEVICE]" << std::endl;

    // Recorder
    // Инициализируем рекордер только при первом запуске
    static bool recorder_initialized = false;
    if (!recorder_initialized) {
        recorder.configure(device.center_freq, device.sample_rate, "../records", 60);
        recorder_initialized = true;
        std::cout << "Успешно инициализирован [RECORDER]" << std::endl;
    }

    // DB
    //db_wrapper.startSever();
    //Сервер запускается вручную через консоль
    // MongoSH
    //db_wrapper.start_mongosh();

    running = true;

    //std::cout << "[WORKER] инициализирован.\n";
}

// 3. Метод установки несущей и ЧД. Пока что захардкожен _____________ в теории можно добавить, чтобы можно было менять ЧД и ЦД
void Worker::set() {
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

// 5. Метод записи сигнала
void Worker::work() {

    std::cout << "[WORKER] Начало асинхронной записи..." << std::endl;

    // DB

    // Device
    device.startRecordingAsync(recorder, db_wrapper);

    // Ждём сигнал остановки
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Останавливаем
    device.stopRecordingAsync();

    std::cout << "[WORKER] Запись остановлена." << std::endl;
    std::cout << "[WORKER] Всего блоков: " << recorder.totalBlocksWritten() << std::endl;
    std::cout << "[WORKER] Последний файл: " << recorder.currentFilename() << std::endl;
}

// 6. Метод остановки записи сигнала
void Worker::stop() {
    {
        running = false; // для остановки work()
        //std::lock_guard<std::mutex> lock(mtx);

        if (worker.joinable()) worker.join(); // Проверяем, что поток существует и ждём реального завершения

        recorder.close();
        db_wrapper.saveData();
        std::cout << "[WORKER] Остановлен. \n";
        std::cout << "[RECORDER] Закрыт. \n";
        //std::cout << "[WORKER] Документов в MongoDB: " << db_wrapper.totalDocuments() << std::endl;
    }
}

// Управление потоками
// joinable() — проверяет, возможно ли присоединение связанного потока.
// join() — блокируется до завершения соответствующего потока.


// 7. Метод вывода статуса БД
void Worker::status() {
    // DB
    db_wrapper.status_mongosh();
}