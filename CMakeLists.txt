set(COMPONENT_SRCS
    "src/rak3172.cpp"
    "src/Commands/rak3172_commands.cpp"
    "src/Commands/rak3172_commands_rui3.cpp"
    "src/Protocols/LoRaWAN/rak3172_lorawan.cpp"
    "src/Protocols/LoRaWAN/rak3172_lorawan_rui3.cpp"
    "src/Protocols/P2P/rak3172_p2p.cpp"
    "src/Protocols/P2P/rak3172_p2p_rui3.cpp"
    )

set(COMPONENT_ADD_INCLUDEDIRS
	"include"
	"include/Protocols"
	"include/Protocols/LoRaWAN"
	"include/Protocols/P2P"
	)

set(COMPONENT_PRIV_REQUIRES freertos)

register_component()