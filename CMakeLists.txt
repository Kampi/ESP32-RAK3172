set(COMPONENT_SRCS
    "src/rak3172.cpp"
    "src/Commands/rak3172_commands.cpp"
    "src/Commands/rak3172_commands_rui3.cpp"
    "src/Modes/LoRaWAN/rak3172_lorawan.cpp"
    "src/Modes/LoRaWAN/rak3172_lorawan_rui3.cpp"
    "src/Modes/P2P/rak3172_p2p.cpp"
    "src/Modes/P2P/rak3172_p2p_rui3.cpp"
    "src/Modes/RF/rak3172_rf.cpp"
    )

set(COMPONENT_ADD_INCLUDEDIRS
	"include"
	)

set(COMPONENT_PRIV_REQUIRES freertos)

register_component()