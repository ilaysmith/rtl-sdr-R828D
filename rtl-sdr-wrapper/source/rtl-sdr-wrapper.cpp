#include "../include/rtl-sdr-wrapper.h"
#include "../../DB-wrapper/include/DB-wrapper.h"
#include <iostream>


// 1. Метод подключения к SDR
int rtlsdr_wrapper::connectionSDR() {

    int device_count = rtlsdr_get_device_count(); // Сканирует usb шин. Возвращает кол-во найденных приёмников. Ищет устройства по VID PID
    if (device_count == 0) {
        std::cerr << "Устройства RTL-SDR не найдены" << std::endl; // стандартный поток ошибок
        return 1;
    }

    // Вывод информации об устройстве
    std::cout << "Найдено устройство: " << device_count << std::endl;
    std::cout << "Имя устройства: "
              << rtlsdr_get_device_name(device_index) << std::endl; // возвращает const char* с описанием устройства.

    // Открытие устройства
    if (rtlsdr_open(&dev, device_index) <
        0) // функция выделит память под структуру rtlsdr_dev_t и запишет её адрес в dev
    {           // При ошибке отрицательное число. Если ошибка, то вывод с ошибкой
        std::cerr << "Ошибка открытия устройства." << std::endl;
    }
    // rtlsdr_open - открывает usd устройство через libusb_open(). Захватывает интерфейс, отправляет команды инициализации, выделяет память под внутренние буферы
    return 0;
}


// 2. Метод установки параметров для SDR (Пока что они заданы жёстко)
void rtlsdr_wrapper::setParameters() {
    int ret;

    // 1. Сначала частота дискретизации
    ret = rtlsdr_set_sample_rate(dev, sample_rate);
    if (ret < 0) std::cerr << "[SDR] Ошибка установки ЧД: " << ret << std::endl;

    // 2. Потом центральная частота
    ret = rtlsdr_set_center_freq(dev, center_freq);
    if (ret < 0) std::cerr << "[SDR] Ошибка установки частоты: " << ret << std::endl;

    // 3. Усиление
    rtlsdr_set_tuner_gain_mode(dev, 0);

    // 4. Ждём стабилизации PLL (R828D нужно больше времени)
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 5. Проверяем, что настроилось
    int actual_freq = rtlsdr_get_center_freq(dev);
    int actual_rate = rtlsdr_get_sample_rate(dev);

    std::cout << "  Частота: " << actual_freq << " Гц (ожидалось " << center_freq << ")" << std::endl;
    std::cout << "  ЧД: " << actual_rate << " Гц (ожидалось " << sample_rate << ")" << std::endl;

    // 6. Сброс буфера
    rtlsdr_reset_buffer(dev);
}


/////////////////////////////// БЛОК АСИНХРОННОГО ЧТЕНИЯ //////////////////////////
// typedef void(*rtlsdr_read_async_cb_t)(unsigned char *buf, uint32_t len, void *ctx);
// void *ctx - указатель на любые данные, которые нужно передать в callback. Используем структуру, так как таких данных несколько, а указатель один


// 2. Статический callback
void rtlsdr_wrapper::iq_callback(unsigned char *buf, uint32_t len, void *ctx_ptr) {
    std::cout << "[CALLBACK] Вызов #" << std::endl;
    // buf — указатель на сырые данные от SDR (I/Q чередующиеся байты), len - длина сырых байтов
    auto *contex = static_cast<Context *>(ctx_ptr); // static_cast преобразует void* обратно в CallbackContext*.

    if (contex->is_streaming->load()) { // атомарная проверка флага. load() - атомарное чтение

        // Запись в файл [RECORDER]
        contex->recorder->save(buf, len);
        // context->recorder — разыменовываем указатель на recorder.
        // write — побайтовая запись в файл (бинарный режим).
        // reinterpret_cast<const char*>(buf) — преобразует unsigned char* в const char*, как требует метод write.
        // len — количество байт для записи.

        std::string filename = contex->recorder->currentFilename(); // Актуальное имя
        std::vector<uint8_t> data(buf, buf + len); // ?????

        // Запись в MongoDB
        contex->db_wrapper->addBlock(88400000, 240000, filename);


        // Увеличиваем счётчик
        contex->counter->fetch_add(1); // fetch_add(1) — атомарное увеличение на 1 (без прерывания)

        std::cout << "Записан блок: " << contex->counter->load() << std::endl;
    }
}

// 3. Асинхронное чтение
void rtlsdr_wrapper::startRecordingAsync(Recorder &recorder, DB_wrapper &db_wrapper) {
    is_streaming = true;

    //rtlsdr_reset_buffer(dev); // reset буфера

    // Создаём контекст для передачи в callback. Структура контекста в динамической памяти. Она будет жить, пока работает асинхронное чтение
    // Локальная переменная уничтожилась бы при выходе из startRecordingAsync
    // Указатели в структуре указывают на поля объекта
    // Сохраняем указатели на объекты

    // Заполняем контекст
    ctx.recorder = &recorder;
    ctx.db_wrapper = &db_wrapper;
    ctx.is_streaming = &is_streaming;
    ctx.counter = &counter_rtl;

    // Запускаем асинхронное чтение в отдельном потоке
    // Создаём новый поток с лямбда функцией. Она нужна, потому что std::thread ожидает функцию, которую нужно выполнить в отдельном потоке. Лямбда позволяет создать такую на месте
    //[this, ctx] — захват переменных:
    // this — чтобы вызвать rtlsdr_read_async как метод объекта.
    // ctx — указатель на контекст (копируется по значению, но это указатель, так что указывать будет на ту же память).
    async_thread = std::thread([this]() {
        // Эта функция блокирует поток до вызова rtlsdr_cancel_async(). Бесконечный цикл
        // iq_callback - статическая функция, передаваемая на каждый блок данных
        // 0, 0 — размеры буфера по умолчанию (15 буферов по 262144 байт каждый).
        rtlsdr_read_async(dev, iq_callback, &this->ctx, 0, 0);
        // &this->ctx — адрес этой структуры

    });

}

// 4. Прерывание асинхронного чтения
void rtlsdr_wrapper::stopRecordingAsync() {
    is_streaming = false;

    // Прерываем асинхронное чтение
    rtlsdr_cancel_async(dev);

    // Ждём завершения потока
    if (async_thread.joinable()) {      // joinable() — проверяет, что поток существует и еще не был join() или detach()
        async_thread.join();        // join() — блокирует текущий поток, пока поток async_thread не завершится (то есть пока rtlsdr_read_async не выйдет и лямбда не выполнит delete ctx
    }
}

rtlsdr_wrapper::rtlsdr_wrapper() {
    std::cout << "[SDR] Конструктор" << std::endl;

}


//                          ПОКА НЕ АКТУАЛЬНО
// 5. Метод чтения сигнала с SDR. Синхронное чтение
int rtlsdr_wrapper::readSignal(std::ofstream &outfile) {

    std::vector<uint8_t> buffer(
            16384); // Динамический массив из 16384 элементов. Все элементы = 0. 16384 размер буфера в байтах
    int n_read = 0; // здесь будет реальное количество бит

    // 1. Запись данных в буфер
    int result = rtlsdr_read_sync(dev, buffer.data(), buffer.size(), &n_read); // !!! НУЖЕН АСИНХРОН


    // 2. Запись данных в файл
    Recorder recorder;
    recorder.saveSignal(outfile, result, n_read, buffer);

    // 3. Запись данных в БД
    // ....

    return 0;
}

