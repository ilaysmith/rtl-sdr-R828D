cmake_minimum_required(VERSION 4.0)
project(SDRtest)

set(CMAKE_CXX_STANDARD 20)

# Поиск библиотеки rtl-sdr через pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(RTLSDR REQUIRED librtlsdr)

# Включение заголовков и линковка
include_directories(${RTLSDR_INCLUDE_DIRS})
link_directories(${RTLSDR_LIBRARY_DIRS})

add_executable(SDRtest main.cpp
        stream/include/Worker.h
        stream/source/Worker.cpp
        Recorder/include/Recorder.h
        Recorder/source/Recorder.cpp
        rtl-sdr-wrapper/source/rtl-sdr-wrapper.cpp
        rtl-sdr-wrapper/include/rtl-sdr-wrapper.h)
target_link_libraries(SDRtest ${RTLSDR_LIBRARIES})
