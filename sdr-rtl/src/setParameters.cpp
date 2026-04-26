#include "../header/RTL_SDR.h"

void RTL_SDR::setParameters() {

    // Настройка параметров приёма

    rtlsdr_set_center_freq(dev, center_freq); // настраивает частоту гетеродина тюнера R828D на указанную частоту
    rtlsdr_set_sample_rate(dev, sample_rate); // Настраивает АЦП RTL2832U на нужную ЧД
    rtlsdr_set_tuner_gain_mode(dev, 0); // Управляет режимом усиления. 0 = авто, 1 = ручной
    // 0 - тюнер сам подстраивает усиление в зависимости от уровня сигнала.
    // 1 - можно задать конкретное усиление через rtlsdr_set_tuner_gain(). Полезно для слабых сигналов

    std::cout << "Приёмник настроен на: " << center_freq << " Гц." << std::endl;
    std::cout << "Приёмник имеет ЧД: " << sample_rate << " Гц." << std::endl;
}