cmake_minimum_required(VERSION 3.16)

set(COMPONENT_SRCS
    "src/rak3172.cpp"
    "src/rak3172_p2p.cpp"
    "src/rak3172_lorawan.cpp"
)

set(COMPONENT_ADD_INCLUDEDIRS
    "include"
)

set(COMPONENT_REQUIRES src)
set(COMPONENT_PRIV_REQUIRES freertos)

register_component()