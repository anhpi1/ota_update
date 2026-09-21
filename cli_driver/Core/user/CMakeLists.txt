
add_library(user_lib OBJECT
    logging/Src/log.c
)


target_include_directories(user_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/logging/Inc
)


target_link_libraries(user_lib PUBLIC
    stm32cubemx
)