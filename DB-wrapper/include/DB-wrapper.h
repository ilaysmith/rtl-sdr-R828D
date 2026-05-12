#ifndef SDRTEST_DB_WRAPPER_H
#define SDRTEST_DB_WRAPPER_H

#include <string>
#include <vector>
#include <cstdint>


class DB_wrapper {
public:
    DB_wrapper();

    // Сохранение данных
    void saveData(int center_freq, int sample_rate,
                  const std::string &filename,
                  int blocks_count, int total_samples);

    // Вывод последних записей через API
    void printLastRecords(int count = 5);

    // Генерация имени для документов
    void addBlock(int center_freq, int sample_rate, const std::string &filename);

    std::string current_filename_;
    int blocks_count_ = 0;
    int center_freq_ = 0;
    int sample_rate_ = 0;

private:
    std::string uri_ = "mongodb://localhost:27017";
    std::string db_name_ = "sdr_data";
    std::string collection_name_ = "signals";
};


#endif //SDRTEST_DB_WRAPPER_H
