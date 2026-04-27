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
        sdr-rtl/src/connectionSDR.cpp
        sdr-rtl/header/RTL_SDR.h
        sdr-rtl/src/setParameters.cpp
        sdr-rtl/src/readSignal.cpp
        stream/header/stream.h
        stream/src/start.cpp
        stream/src/work.cpp
        stream/src/stop.cpp
        stream/src/init.cpp
        stream/src/set.cpp
        stream/src/exit.cpp)
target_link_libraries(SDRtest ${RTLSDR_LIBRARIES})
