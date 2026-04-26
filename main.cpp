#include "sdr-rtl/header/RTL_SDR.h"


int main() {

    RTL_SDR device;

    device.connectionSDR();
    device.setParameters();
    device.readSignal();
    device.stopRecording();


    return 0;
}



// int result = rtlsdr_read_sync(dev, buffer.data(), buffer.size(), &n_read);
// dev - указатель на устройство
// buffer.data() - указатель на начало выделенной памяти
// buffer.size() - количество байт, которое хотим прочитать
// &n_read - адрес переменной, куда запишется реальное число количества прочитанных бит
// Функция блокирует выполнение программы (ждёт).
// Ждёт, пока с USB накопится нужное количество байт
// Копирует данные из внутреннего буфера драйвера в buffer
// Возвращает управления программы

// rtlsdr_close() освобождает usb-интерфейс, закрывает соединение, высвобождает всю память, выделенную внутри rtlsdr_open()
// Без закрытия приёмник может остаться в захваченном состоянии

























