#ifndef SDRTEST_DB_WRAPPER_H
#define SDRTEST_DB_WRAPPER_H

#include <string>
#include <vector>
#include <cstdint>


class DB_wrapper {
public:
    DB_wrapper();

    void saveIQData(const std::vector<uint8_t> &data, int center_freq, int sample_rate, const std::string &filename);

    size_t totalDocuments();

    bool startSever();

    bool stopServer();

    bool isServerRunning();

    // Mongosh
    bool start_mongosh();

    bool status_mongosh();

    // Запись данных не по колбэку, а с накоплением
    std::vector<std::vector<uint8_t>> buffer_;  // накопленные блоки
    int buffer_center_freq_ = 0;
    int buffer_sample_rate_ = 0;
    std::string buffer_filename_;
    size_t max_blocks_ = 60;   // лимит блоков на документ
    size_t blocks_saved_ = 0;   // всего сохранено


    void addBlock(const std::vector<uint8_t> &data, int center_freq, int sample_rate, const std::string &filename);

    void saveData();  // принудительно сохранить накопленные блоки
};


#endif //SDRTEST_DB_WRAPPER_H
