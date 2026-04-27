#include <iostream>
#include "../header/stream.h"

void BackgroundWorker::exit() {

    device.stopRecording();
    std::cout << "[WORKER] Устройство отключено. \n";
}
