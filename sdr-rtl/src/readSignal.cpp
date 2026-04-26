#include "../header/RTL_SDR.h"

int RTL_SDR::readSignal() {

    rtlsdr_reset_buffer(
            dev); // сброс usb буфера - чтобы не вернуть старые данные до настройки частоты, так как приемник работает непрерывно с момента открытия

    std::cout << "Чтение 16384 байт данных..." << std::endl;

    // Запись данных в файл
    std::ofstream outfile("../iq_recording.bin", std::ios::binary);
    if (!outfile) {
        std::cerr << "Ошибка создания файла!" << std::endl;
        rtlsdr_close(dev);
        return 1;
    }

    std::cout << "Начало записи в файл iq_recording.bin. Для выхода нажать на пиктограмму прекращения работы"
              << std::endl;

    std::vector<uint8_t> buffer(
            16384); // Динамический массив из 16384 элементов. Все элементы = 0. 16384 размер буфера в байтах
    int n_read = 0; // здесь будет реальное количество бит

    // Читаем и пишем
    // Запись 10 секунд данных
    int total_seconds = 10;
    long long total_samples = sample_rate * total_seconds;
    long long samples_read = 0;

    while (samples_read < total_samples) {
        int result = rtlsdr_read_sync(dev, buffer.data(), buffer.size(), &n_read);
        if (result >= 0 && n_read > 0) {
            outfile.write(reinterpret_cast<const char *>(buffer.data()), n_read);
            std::cout << "Успешно прочитано и записано " << n_read << " байт." << std::endl;
            samples_read += n_read / 2; // Делим на 2, т.к. каждый сэмпл — 2 байта (I+Q)
        } else {
            std::cerr << "Ошибка чтения данных." << std::endl;
            break;
        }
    }
    outfile.close();
    return 0;
}