#ifndef SDRTEST_RTL_SDR_WRAPPER_H
#define SDRTEST_RTL_SDR_WRAPPER_H

#include <iostream>
#include <vector>
#include <rtl-sdr.h>
#include <fstream> // Для работы с файлами
#include <atomic>
#include <thread> // Для работы с потоками
#include "../../Recorder/include/Recorder.h"

class rtlsdr_wrapper {
public:
    uint32_t device_index = 0; //  индекс для обращения к устройству
    rtlsdr_dev_t *dev = nullptr; // указатель на непрозрачную структуру.

    int center_freq = 88400000; // FM-станции вещают в диапазоне 87.5-108 МГц.
    int sample_rate = 2048000; // RTL-SDR аппаратно поддерживает несколько фиксированных значений. Библиотека автоматически округляет до ближайшего

    // Асинхронное чтение с rtl sdr
    std::thread async_thread;
    std::atomic<bool> is_streaming{false};
    std::atomic<uint64_t> counter_rtl;

    void startRecordingAsync(Recorder &recorder);

    void stopRecordingAsync();

    static void iq_callback(unsigned char *buf, uint32_t len, void *ctx); // статик
    // Обычный метод класса неявно принимает this (указатель на объект). Он доступен, но сигнатура не совпадает с С-функцией
    // C ожидает: void(*)(unsigned char*, uint32_t, void*)
    // C++ метод:  void(rtlsdr_wrapper*, unsigned char*, uint32_t, void*)
    // Статический метод не имеет скрытого this, поэтому его сигнатура совпадает с С-функцией и такой метод можно передать в rtlsdr_read_async
    // Но со static нет доступа к this->dev и другим полям объекта
    // Всё, что нужно колбэку, передаём через void *ctx, так как this-> недоступен

    //

    rtlsdr_wrapper() {}

    int connectionSDR();

    void setParameters();

    int readSignal(std::ofstream &outfile);

    void stopRecording() {
        rtlsdr_close(dev);
    };
};


#endif //SDRTEST_RTL_SDR_WRAPPER_H
