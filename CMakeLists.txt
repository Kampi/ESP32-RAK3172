set(COMPONENT_SRCS
    "src/rak3172.cpp"
    "src/Commands/rak3172_commands.cpp"
    "src/Commands/rak3172_commands_rui3.cpp"
    "src/Modes/Private/rak3172_tools.cpp"
    )

set(COMPONENT_ADD_INCLUDEDIRS
	"include"
	"include/Modes"
	"include/Definitions"
	)

if(CONFIG_RAK3172_MODE_WITH_LORAWAN)
    list(APPEND COMPONENT_SRCS "src/Modes/LoRaWAN/rak3172_lorawan.cpp")
    list(APPEND COMPONENT_SRCS "src/Modes/LoRaWAN/rak3172_lorawan_rui3.cpp")
    list(APPEND COMPONENT_SRCS "src/Modes/LoRaWAN/rak3172_lorawan_multicast.cpp")
    list(APPEND COMPONENT_SRCS "src/Modes/LoRaWAN/rak3172_lorawan_class_b.cpp")

    if(CONFIG_RAK3172_MODE_WITH_LORAWAN_FUOTA)
        list(APPEND COMPONENT_SRCS "src/Modes/LoRaWAN/FUOTA/rak3172_lorawan_fuota.cpp")
        list(APPEND COMPONENT_SRCS "src/Modes/LoRaWAN/FUOTA/Semtech/FragDecoder.c")
        list(APPEND COMPONENT_SRCS "src/Modes/LoRaWAN/FUOTA/Semtech/utilities.c")
    endif()

    if(CONFIG_RAK3172_MODE_WITH_LORAWAN_CLOCK_SYNC)
        list(APPEND COMPONENT_SRCS "src/Modes/LoRaWAN/ClockSynchronization/rak3172_lorawan_clock_sync.cpp")
    endif()
endif()

if(CONFIG_RAK3172_MODE_WITH_P2P)
    list(APPEND COMPONENT_SRCS "src/Modes/P2P/rak3172_p2p.cpp")
    list(APPEND COMPONENT_SRCS "src/Modes/P2P/rak3172_p2p_rui3.cpp")
endif()

if((IDF_TARGET STREQUAL "esp32") OR (IDF_TARGET STREQUAL "esp32c2") OR (IDF_TARGET STREQUAL "esp32c3") OR (IDF_TARGET STREQUAL "esp32s2") OR (IDF_TARGET STREQUAL "esp32s3"))
	list(APPEND COMPONENT_PRIV_REQUIRES esp_timer)
	list(APPEND COMPONENT_SRCS "src/Arch/Timer/rak3172_timer.cpp")
	list(APPEND COMPONENT_SRCS "src/Arch/PwrMgmt/rak3172_pwrmgmt.cpp")
	list(APPEND COMPONENT_SRCS "src/Arch/Flash/rak3172_flash.cpp")
	list(APPEND COMPONENT_SRCS "src/Arch/UART/rak3172_uart.cpp")
	list(APPEND COMPONENT_SRCS "src/Arch/GPIO/rak3172_gpio.cpp")
endif()

idf_component_register(SRCS ${COMPONENT_SRCS}
                       INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}

                       # Only public requires, because the header files includes them
                       REQUIRES freertos driver

                       # The private requires are only used by the driver
                       PRIV_REQUIRES ${COMPONENT_PRIV_REQUIRES}
                       )

target_compile_definitions(${COMPONENT_LIB} PUBLIC RAK3172_LIB_MAJOR=4)
target_compile_definitions(${COMPONENT_LIB} PUBLIC RAK3172_LIB_MINOR=2)
target_compile_definitions(${COMPONENT_LIB} PUBLIC RAK3172_LIB_BUILD=1)