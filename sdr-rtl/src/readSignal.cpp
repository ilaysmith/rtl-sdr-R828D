#include "../header/RTL_SDR.h"

int RTL_SDR::readSignal(std::ofstream &outfile) {

    std::vector<uint8_t> buffer(
            16384); // Динамический массив из 16384 элементов. Все элементы = 0. 16384 размер буфера в байтах
    int n_read = 0; // здесь будет реальное количество бит

    int result = rtlsdr_read_sync(dev, buffer.data(), buffer.size(), &n_read);
    if (result >= 0 && n_read > 0) {
        outfile.write(reinterpret_cast<const char *>(buffer.data()), n_read);
        //std::cout << "Успешно прочитано и записано " << n_read << " байт." << std::endl;
    } else {
        std::cerr << "Ошибка чтения данных." << std::endl;
        return -1;
    }

    return 0;
}



/*
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
 */