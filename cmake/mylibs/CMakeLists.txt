cmake_minimum_required(VERSION 3.22)
enable_language(C)

set(lib_name mylibs)
set(lib_include_path ../../mylibs/include)
set(lib_source_path ../../mylibs/source)

add_library(${lib_name} STATIC)

set(fatfs_source 
    ${lib_source_path}/../fatfs/diskio.c
    ${lib_source_path}/../fatfs/ff.c
    ${lib_source_path}/../fatfs/ffsystem.c
    ${lib_source_path}/../fatfs/ffunicode.c
)
set(oled_source
    ${lib_source_path}/mod_oled.c
    ${lib_source_path}/lib_font_fixedsys.c
)
target_sources(${lib_name} PRIVATE
    ${lib_source_path}/lib_usart.c
    ${lib_source_path}/lib_tool.c
    ${lib_source_path}/lib_spi.c
    ${lib_source_path}/mod_flash.c
    ${lib_source_path}/lib_rtc.c
    ${lib_source_path}/lib_i2c.c
    ${lib_source_path}/mod_dht11.c
    ${lib_source_path}/lib_timer.c
    ${lib_source_path}/task.c
    ${oled_source}
    ${fatfs_source}
)
target_include_directories(${lib_name}
    PUBLIC ${lib_include_path}/.
    PUBLIC ${lib_include_path}/../fatfs
)
target_link_libraries(${lib_name} PRIVATE STM32_Drivers)