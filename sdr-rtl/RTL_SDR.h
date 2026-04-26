#ifndef SDRTEST_RTL_SDR_H
#define SDRTEST_RTL_SDR_H

#include <iostream>
#include <vector>
#include <rtl-sdr.h>
#include <fstream> // Для работы с файлами

class RTL_SDR {
public:
    uint32_t device_index = 0; //  индекс для обращения к устройству
    rtlsdr_dev_t *dev = nullptr; // указатель на непрозрачную структуру.

    int center_freq = 88400000; // FM-станции вещают в диапазоне 87.5-108 МГц.
    int sample_rate = 2048000; // RTL-SDR аппаратно поддерживает несколько фиксированных значений. Библиотека автоматически округляет до ближайшего


    RTL_SDR() {}

    int connectionSDR();

    void setParameters();

    int readSignal();

    void stopRecording() {
        rtlsdr_close(dev);
    };
};


#endif //SDRTEST_RTL_SDR_H
