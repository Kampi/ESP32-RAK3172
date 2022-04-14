set(COMPONENT_SRCS
    "src/rak3172.cpp"
    "src/rak3172_lorawan.cpp"
    "src/rak3172_p2p.cpp"
    )

set(COMPONENT_ADD_INCLUDEDIRS
	"include"
	)

set(COMPONENT_PRIV_REQUIRES freertos)

register_component()