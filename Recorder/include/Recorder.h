#ifndef SDRTEST_RECORDER_H
#define SDRTEST_RECORDER_H

#include <iostream>
#include <fstream> // Для работы с файлами
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

class Recorder {
public:

    // Создать новый файл
    void openNewFile();

    // Сгенерировать имя файла
    std::string generateFilename(int file_number);

    int center_freq_;            // Центральная частота
    int sample_rate_;             // ЧД
    size_t max_blocks_ = 100;               // Максимальное количество блоков в файле
    size_t blocks_in_current_file_ = 0;     // Блоков в текущем файле
    size_t file_counter_ = 0;               // Счётчик файлов
    size_t total_blocks_ = 0;               // Всего блоков записано

    std::ofstream outfile_;         // Текущий файловый поток
    std::string current_filename_;  // Имя текущего файла
    std::string output_dir_;  // Дял сохранения в нужную папку

    // Настройка параметров
    void configure(int center_freq, int sample_rate, const std::string &output_dir, size_t max_blocks_per_file = 100);

    // Запись блока данных
    int save(unsigned char *buffer, size_t len);

    // Закрыть текущий файл
    void close();

    // Получить информацию о файле
    std::string currentFilename() const { return current_filename_; }

    size_t totalBlocksWritten() const { return total_blocks_; }

    size_t currentFileNumber() const { return file_counter_; }

    Recorder() {}

    int saveSignal(std::ofstream &outfile, int &flag, int &n_read, std::vector<uint8_t> &buffer);


    void openFile();

    ~Recorder() {}
};

#endif //SDRTEST_RECORDER_H
