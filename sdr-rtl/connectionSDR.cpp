#include "../sdr-rtl//RTL_SDR.h"

int RTL_SDR::connectionSDR() {

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