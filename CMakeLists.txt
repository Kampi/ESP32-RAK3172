set(COMPONENT_SRCS
    "src/rak3172.cpp"
    "src/Commands/rak3172_commands.cpp"
    "src/Commands/rak3172_commands_rui3.cpp"
    "src/Modes/LoRaWAN/rak3172_lorawan.cpp"
    "src/Modes/LoRaWAN/rak3172_lorawan_rui3.cpp"
    "src/Modes/LoRaWAN/rak3172_lorawan_multicast.cpp"
    "src/Modes/LoRaWAN/rak3172_lorawan_class_b.cpp"
    "src/Modes/LoRaWAN/rak3172_lorawan_fota.cpp"
    "src/Modes/P2P/rak3172_p2p.cpp"
    "src/Modes/P2P/rak3172_p2p_rui3.cpp"
    "src/Modes/RF/rak3172_rf.cpp"
    )

set(COMPONENT_ADD_INCLUDEDIRS
	"include"
	"include/Modes"
	"include/Definitions"
	)

set(COMPONENT_PRIV_REQUIRES freertos driver)

if((IDF_TARGET STREQUAL "esp32") OR (IDF_TARGET STREQUAL "esp32c2") OR (IDF_TARGET STREQUAL "esp32c3") OR (IDF_TARGET STREQUAL "esp32s2") OR (IDF_TARGET STREQUAL "esp32s3"))
	list(APPEND COMPONENT_PRIV_REQUIRES esp_timer)
	list(APPEND COMPONENT_SRCS 	"src/Arch/Timer/rak3172_timer.cpp")
	list(APPEND COMPONENT_SRCS 	"src/Arch/PwrMgmt/rak3172_pwrmgmt.cpp")
endif()

register_component()
