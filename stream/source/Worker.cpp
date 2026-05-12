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

// 2. Метод инициализации
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

    running = true;
}

// 3. Метод установки несущей и ЧД
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

    //worker = std::thread(&Worker::work, this); // Создаём поток -> вызов work.cpp -> readSignal
    // в конструктор передаём указатель на метод класса, this - указатель на текущий объект класса, который вызвал start()
    device.startRecordingAsync(recorder, db_wrapper);

    std::cout << "[WORKER] Запущен.\n" << std::endl;
}


// 6. Метод остановки записи сигнала
void Worker::stop() {
    {
        running = false; // для остановки work()
        //std::lock_guard<std::mutex> lock(mtx);

        device.stopRecordingAsync();
        recorder.close();

        // Сохраняем метаданные для последнего файла
        db_wrapper.saveData(
                device.center_freq, device.sample_rate,
                recorder.currentFilename(),
                recorder.blocks_in_current_file_,
                recorder.blocks_in_current_file_ * 262144 / 2
        );

        std::cout << "[WORKER] Остановлен. \n";
    }
}

// Управление потоками
// joinable() — проверяет, возможно ли присоединение связанного потока.
// join() — блокируется до завершения соответствующего потока.


// 7. Метод вывода статуса БД
void Worker::status() {
    // DB
    db_wrapper.printLastRecords();
}

//////////// НЕ АКТУАЛЬНО //////////
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