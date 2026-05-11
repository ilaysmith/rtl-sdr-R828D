#ifndef SDRTEST_RECORDER_H
#define SDRTEST_RECORDER_H

#include <iostream>
#include <fstream> // Для работы с файлами
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include "../../DB-wrapper/include/DB-wrapper.h"

class Recorder {
public:
    Recorder();

    ~Recorder() {}

    // Параметры для configure
    int center_freq_;            // Центральная частота
    int sample_rate_;             // ЧД

    // Параметры для вывода и учета статистики блоков в файл (не больше 100 блоков в файл)
    size_t max_blocks_ = 60;               // Максимальное количество блоков в файле
    size_t blocks_in_current_file_ = 0;     // Блоков в текущем файле
    size_t file_counter_ = 0;               // Счётчик файлов
    size_t total_blocks_ = 0;               // Всего блоков записано

    // Параметры для записи сигнала в файловый поток
    std::ofstream outfile_;         // Текущий файловый поток
    std::string current_filename_;  // Имя текущего файла
    std::string output_dir_;  // Дял сохранения в нужную папку

    // Настройка параметров
    void configure(int center_freq, int sample_rate, const std::string &output_dir, size_t max_blocks_per_file = 100);

    // Сгенерировать имя файла
    std::string generateFilename(int file_number);

    // Запись блока данных. Новый формат
    int save(unsigned char *buffer, size_t len);

    // Закрыть текущий файл
    void close();

    // Открыть новый файл. В случае, если запись включена повторно
    void openNewFile();

    // Получить информацию о файле
    std::string currentFilename() const { return current_filename_; }

    size_t totalBlocksWritten() const { return total_blocks_; }

    size_t currentFileNumber() const { return file_counter_; }

    // Запись сигнала в файл. Старый формат
    int saveSignal(std::ofstream &outfile, int &flag, int &n_read, std::vector<uint8_t> &buffer);

};

#endif //SDRTEST_RECORDER_H
