#include <iostream>      // Для cout, cin (консоль)
#include "../header/stream.h"

void BackgroundWorker::work() {

    // Запись данных в файл
    std::ofstream outfile("../iq_recording.bin", std::ios::binary);
    if (!outfile) {
        std::cerr << "Ошибка создания файла!" << std::endl;
        rtlsdr_close(device.dev);
        return;
    }

    std::cout << "Начало записи в файл iq_recording.bin. Для выхода нажать на пиктограмму прекращения работы"
              << std::endl;

    std::cout << "[WORKER] Начало записи..." << std::endl;

    rtlsdr_reset_buffer(
            device.dev); // сброс usb буфера - чтобы не вернуть старые данные до настройки частоты, так как приемник работает непрерывно с момента открытия

    while (running) { // атомарный флаг
        //std::lock_guard<std::mutex> lock(mtx); // Защищаем доступ
        //if (!running) break; // Если просили остановиться - выходим

        device.readSignal(outfile);
        ++counter;

    } // Здесь замок автоматически снимается

    outfile.close();
    std::cout << "[WORKER] Запись остановлена. Счетчик: " << counter << std::endl;

}