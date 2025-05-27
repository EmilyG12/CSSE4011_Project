#set(BOARD m5stack_core2/esp32/procpu)
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(artemis-lime-trainer)

target_include_directories(app PRIVATE ../libs)
target_sources(app PRIVATE
        ../libs/bt/bluetooth.c
        ../libs/bt/bt_scanner.c
        ../libs/bt/bt_advertiser.c
        #        ../../libs/bt/cli_observer.c
        ../libs/keypad.c
)

file(GLOB IMG_FILES ${PROJECT_SOURCE_DIR}/src/ui/images/*.c)
if (DEFINED MY_UUID)
    target_compile_definitions(app PRIVATE MY_UUID=${MY_UUID})
endif()

target_include_directories(app PRIVATE src)
target_sources(app PRIVATE
        src/fight_ad.c
        src/game_controller.c
        src/controller.c
        src/game.c
        src/fight.c
        src/user.c
        src/pokedex.c
        src/main.c

        src/ui/viewer.c
        ${IMG_FILES}
)
