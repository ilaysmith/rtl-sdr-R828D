#ifndef SDRTEST_DB_WRAPPER_H
#define SDRTEST_DB_WRAPPER_H

#include <string>
#include <vector>
#include <cstdint>


class DB_wrapper {
public:
    DB_wrapper();

    void saveIQData(const std::vector<uint8_t> &data, int center_freq, int sample_rate);

    size_t totalDocuments();

    bool startSever();

    bool stopServer();

    bool isServerRunning();

    // Mongosh
    bool start_mongosh();

    bool status_mongosh();
};


#endif //SDRTEST_DB_WRAPPER_H
