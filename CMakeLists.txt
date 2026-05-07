cmake_minimum_required(VERSION 3.13)
project(SDRtest)

set(CMAKE_CXX_STANDARD 17)

#------------------------------------------------------------------------
# Путь к драйверам MongoDB
set(MONGO_ROOT /home/Ilya/mongo_drivers) # /home/$ENV{USER}/mongo_drivers

# Сообщаем CMake, где искать библиотеки и заголовки
list(APPEND CMAKE_PREFIX_PATH ${MONGO_ROOT})

# Находим библиотеки MongoDB
find_package(libbsoncxx REQUIRED)
find_package(libmongocxx REQUIRED)
#-------------------------------------------------------------------------


# Поиск библиотеки rtl-sdr через pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(RTLSDR REQUIRED librtlsdr)

# Включение заголовков и линковка RTL
include_directories(${RTLSDR_INCLUDE_DIRS})
link_directories(${RTLSDR_LIBRARY_DIRS})


add_executable(SDRtest main.cpp
        stream/include/Worker.h
        stream/source/Worker.cpp
        Recorder/include/Recorder.h
        Recorder/source/Recorder.cpp
        rtl-sdr-wrapper/source/rtl-sdr-wrapper.cpp
        rtl-sdr-wrapper/include/rtl-sdr-wrapper.h
        DB-wrapper/include/DB-wrapper.h
        DB-wrapper/source/DB-wrapper.cpp)


# Заголовки
target_include_directories(SDRtest PRIVATE
        ${RTLSDR_INCLUDE_DIRS}
        /home/Ilya/mongo_drivers/include/mongocxx/v_noabi
        /home/Ilya/mongo_drivers/include/bsoncxx/v_noabi
)

# Линковка MongoDB и RTL
target_link_libraries(SDRtest PRIVATE
        ${RTLSDR_LIBRARIES}
        /home/Ilya/mongo_drivers/lib/libmongocxx.so
        /home/Ilya/mongo_drivers/lib/libbsoncxx.so)




