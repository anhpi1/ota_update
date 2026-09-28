add_library(user_lib OBJECT)

# Khối service (Dùng target_sources để nạp file .c)
target_sources(user_lib PRIVATE
    service/logging/serLog.c
    service/cli/serCli.c
)

# Khối app (Khi nào có file thì mở ra dùng)
target_sources(user_lib PRIVATE
    app/startup.c
)

# Khối driver (Khi nào có file thì mở ra dùng)
target_sources(user_lib PRIVATE
    driver/uart/drvUart.c
)

# Khối test
target_sources(user_lib PRIVATE
    tester/testLog.c
)


# Thư mục gốc để include dạng #include "service/logging/serLog.h"
target_include_directories(user_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(user_lib PUBLIC
    stm32cubemx
)