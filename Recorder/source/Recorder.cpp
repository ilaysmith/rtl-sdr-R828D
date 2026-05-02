#include "../include/Recorder.h"
#include <iostream>
#include <fstream> // Для работы с файлами
#include <vector>
#include <ctime>

int Recorder::saveSignal(std::ofstream &outfile, int &flag, int &n_read, std::vector<uint8_t> &buffer) {

    if (flag >= 0 && n_read > 0) {
        outfile.write(reinterpret_cast<const char *>(buffer.data()), n_read);
        //std::cout << "Успешно прочитано и записано " << n_read << " байт." << std::endl;
    } else {
        std::cerr << "Ошибка чтения данных." << std::endl;
        return -1;
    }
    return 0;

}

void Recorder::configure(int center_freq, int sample_rate, const std::string &output_dir, size_t max_blocks_per_file) {
    center_freq_ = center_freq;
    sample_rate_ = sample_rate;
    max_blocks_ = max_blocks_per_file;
    output_dir_ = output_dir;
    file_counter_ = 0;
    total_blocks_ = 0;
    blocks_in_current_file_ = 0;
}

std::string Recorder::generateFilename(int file_number) {
    // Формат FM{частота}_{ЧД}_{номер}.bin
    std::ostringstream oss;     // Поток для записи в строку (в память)
    oss << output_dir_ << "/"
        << "FM"
        << center_freq_
        << "_"
        << sample_rate_
        << "_"
        << std::setfill('0') << std::setw(3) << file_number // заполнить пустые места "0", ширина поля 3 символа
        << ".bin";
    return oss.str();
}

void Recorder::openNewFile() {
    // Закрываем предыдущий файл, если он открыт
    if (outfile_.is_open()) {
        outfile_.close();
        std::cout << "[RECORDER] Закрыт файл:" << current_filename_ << " | Блоков: " << blocks_in_current_file_
                  << std::endl;
    }

    // Генерируем имя для нового файла
    current_filename_ = generateFilename(file_counter_++);

    // Открываем новый файл
    outfile_.open(current_filename_, std::ios::binary);
    if (!outfile_) {
        std::cerr << "[RECORDER] Ошибка создания файла: " << current_filename_ << std::endl;
        return;
    }

    blocks_in_current_file_ = 0;
    std::cout << "[RECORDER] Создан файл: " << current_filename_ << std::endl;
}

int Recorder::save(unsigned char *buffer, size_t len) {
    // Если файл не открыт или превышен лимит - создаём новый
    if (!outfile_.is_open() || blocks_in_current_file_ >= max_blocks_) {
        openNewFile();
    }

    if (!outfile_) {
        std::cerr << "[RECORDER] Файл не открыт, данные не записаны" << std::endl;
        return -1;
    }

    // Пишем блок
    outfile_.write(reinterpret_cast<const char *>(buffer), len);

    // Обновляем счётчик
    blocks_in_current_file_++;
    total_blocks_++;

    return 0;
}

void Recorder::close() {
    if (outfile_.is_open()) {
        outfile_.close();
        std::cout << "[RECORDER] Закрыт файл:" << current_filename_ << " | Всего блоков: " << total_blocks_
                  << std::endl;
    }
}


