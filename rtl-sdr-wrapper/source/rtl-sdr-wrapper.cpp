#include "../include/rtl-sdr-wrapper.h"
#include "../../Recorder/include/Recorder.h"
#include <iostream>

// 1. Метод подключения к SDR
int rtlsdr_wrapper::connectionSDR() {

    int device_count = rtlsdr_get_device_count(); // Сканирует usb шин. Возвращает кол-во найденных приёмников. Ищет устройства по VID PID
    if (device_count == 0) // Если устройств нет
    {
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


// 2. Метод чтения сигнала с SDR. Синхронное чтение -------- не актуально
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


// 3. Метод установки параметров для SDR (Пока что они заданы жёстко)
void rtlsdr_wrapper::setParameters() {

    // Настройка параметров приёма

    rtlsdr_set_center_freq(dev, center_freq); // настраивает частоту гетеродина тюнера R828D на указанную частоту
    rtlsdr_set_sample_rate(dev, sample_rate); // Настраивает АЦП RTL2832U на нужную ЧД
    rtlsdr_set_tuner_gain_mode(dev, 0); // Управляет режимом усиления. 0 = авто, 1 = ручной
    // 0 - тюнер сам подстраивает усиление в зависимости от уровня сигнала.
    // 1 - можно задать конкретное усиление через rtlsdr_set_tuner_gain(). Полезно для слабых сигналов

    //std::cout << "Приёмник настроен на: " << center_freq << " Гц." << std::endl;
    //std::cout << "Приёмник имеет ЧД: " << sample_rate << " Гц." << std::endl;
}

///////////////////////////////
// typedef void(*rtlsdr_read_async_cb_t)(unsigned char *buf, uint32_t len, void *ctx);
// void *ctx - указатель на любые данные, которые нужно передать в callback. Используем структуру, так как таких данных несколько, а указатель один
////////////////////////////
struct CallbackContex {
    Recorder *recorder;
    //std::ofstream *outfile; // указатель на файловый поток
    std::atomic<bool> *is_streaming; // указатель на флаг, по которому callback понимает, продолжать ли запись
    std::atomic<uint64_t> *counter_rtl; // Для подсчёта блоков, чтобы увеличивать его внутри callback
};


// Статический callback
void rtlsdr_wrapper::iq_callback(unsigned char *buf, uint32_t len, void *ctx) {
    std::cout << "[CALLBACK] Вызов #" << std::endl;
    // buf — указатель на сырые данные от SDR (I/Q чередующиеся байты), len - их длина
    auto *contex = static_cast<CallbackContex *>(ctx); // static_cast преобразует void* обратно в CallbackContext*.

    if (contex->is_streaming->load()) { // атомарная проверка флага. load() - атомарное чтение
        // Пишем данные в файл
        contex->recorder->save(buf, len);
        // context->outfile — разыменовываем указатель на ofstream.
        // write — побайтовая запись в файл (бинарный режим).
        // reinterpret_cast<const char*>(buf) — преобразует unsigned char* в const char*, как требует метод write.
        // len — количество байт для записи.

        // Увеличиваем счётчик
        if (contex->counter_rtl) {      // Проверяем, что указатель на счетчик не nullptr
            contex->counter_rtl->fetch_add(1); // fetch_add(1) — атомарное увеличение на 1 (без прерывания).
        }
        std::cout << "Записан блок: " << contex->counter_rtl->load() << std::endl;
    }
}

void rtlsdr_wrapper::startRecordingAsync(Recorder &recorder) {
    is_streaming = true;

    rtlsdr_reset_buffer(dev);

    std::cout << "[SDR] Проверка настроек:" << std::endl;
    std::cout << "  Частота: " << rtlsdr_get_center_freq(dev) << " Гц" << std::endl;
    std::cout << "  ЧД: " << rtlsdr_get_sample_rate(dev) << " Гц" << std::endl << std::endl;

    // Создаём контекст для передачи в callback. Структура контекста в динамической памяти. Она будет жить, пока работает асинхронное чтение
    // Локальная переменная уничтожилась бы при выходе из startRecordingAsync
    // Указатели в структуре указывают на поля объекта и на внешний outfile - они становятся валидными
    auto *ctx = new CallbackContex{
            &recorder,
            &is_streaming,
            &counter_rtl
    };

    // Запускаем асинхронное чтение в отдельном потоке
    // Создаём новый поток с лямбда функцией. Она нужна, потому что std::thread ожидает функцию, которую нужно выполнить в отдельном потоке. Лямбда позволяет создать такую на месте
    //[this, ctx] — захват переменных:
    // this — чтобы вызвать rtlsdr_read_async как метод объекта.
    // ctx — указатель на контекст (копируется по значению, но это указатель, так что указывать будет на ту же память).
    async_thread = std::thread([this, ctx]() {
        // Эта функция блокирует поток до вызова rtlsdr_cancel_async(). Бесконечный цикл
        // iq_callback - статическая функция, передаваемая на каждый блок данных
        // 0, 0 — размеры буфера по умолчанию (15 буферов по 262144 байт каждый).
        rtlsdr_read_async(dev, iq_callback, ctx, 0, 0);
        std::cout << "[ASYNC] rtlsdr_read_async вернула: " << std::endl;
        delete ctx; // очищаем контекст после завершения
    });

}

void rtlsdr_wrapper::stopRecordingAsync() {
    is_streaming = false;

    // Прерываем асинхронное чтение
    rtlsdr_cancel_async(dev);

    // Ждём завершения потока
    if (async_thread.joinable()) {      // joinable() — проверяет, что поток существует и еще не был join() или detach()
        async_thread.join();        // join() — блокирует текущий поток, пока поток async_thread не завершится (то есть пока rtlsdr_read_async не выйдет и лямбда не выполнит delete ctx
    }
}