# thêm các dòng sau vào <Thư mục dự án>/CMakeLists.txt 

# add_subdirectory(Core/user)
# target_link_libraries(${CMAKE_PROJECT_NAME}
#     service_lib
#     app_lib
# )

# và dòng sau ở sau dòng enable_language(C ASM)
# ghi dòng này 
# set(CMAKE_C_FLAGS_DEBUG "-Os -g3" CACHE STRING "" FORCE)
# add_compile_options(-Os)


# ==========================================
# 1. THƯ VIỆN SERVICE & DRIVER (Nằm ở FLASH_SERVICE)
# ==========================================
add_library(service_lib OBJECT)

target_sources(service_lib PRIVATE
    # Khối service
    service/logging/serlog.c
    service/cli/sercli.c
    service/backup/serbackup.c
    service/ota/serota.c

    # Khối driver
    driver/uart/drvuart.c
    driver/flash/drvflash.c
    driver/crc/drvcrc.c

    # Khối bootloader
    bootloader.s
    bootloader/bootloader.c
    bootloader/sha/sha256.c

    # Khối test (nếu có)
    tester/testLog.c

)

target_include_directories(service_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(service_lib PUBLIC
    stm32cubemx
)

# ==========================================
# 2. THƯ VIỆN APP (Nằm ở FLASH_APP1)
# ==========================================
add_library(app_lib OBJECT)

target_sources(app_lib PRIVATE
    app/start/appstart.c
    app/start/command.c
    app/appvectors.s 
)

target_include_directories(app_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(app_lib PUBLIC
    stm32cubemx
    service_lib
)

add_subdirectory(hook)
target_link_libraries(${CMAKE_PROJECT_NAME} hook_lib)

# ==============================================================================
# CẤU HÌNH BẺ LÁI HAL WEAK CHO SERVICE & CHỐNG LỖI BUILTIN BOOTLOADER
# ==============================================================================
# 1. Bẻ lái trực tiếp toàn bộ 30 hàm weak của STM32_Drivers sang __wrap_* qua cờ -D (Zero Code Touch)
target_compile_definitions(STM32_Drivers PRIVATE
    # HAL MspInit & Timebase
    HAL_CRC_MspInit=__wrap_HAL_CRC_MspInit
    HAL_CRC_MspDeInit=__wrap_HAL_CRC_MspDeInit
    HAL_UART_MspInit=__wrap_HAL_UART_MspInit
    HAL_UART_MspDeInit=__wrap_HAL_UART_MspDeInit
    HAL_TIM_PeriodElapsedCallback=__wrap_HAL_TIM_PeriodElapsedCallback
    HAL_MspInit=__wrap_HAL_MspInit
    HAL_InitTick=__wrap_HAL_InitTick
    HAL_SuspendTick=__wrap_HAL_SuspendTick
    HAL_ResumeTick=__wrap_HAL_ResumeTick

    # HAL Peripheral Callbacks
    HAL_GPIO_EXTI_Callback=__wrap_HAL_GPIO_EXTI_Callback
    HAL_UART_RxCpltCallback=__wrap_HAL_UART_RxCpltCallback
    HAL_UART_TxCpltCallback=__wrap_HAL_UART_TxCpltCallback
    HAL_UART_ErrorCallback=__wrap_HAL_UART_ErrorCallback
    HAL_UART_RxHalfCpltCallback=__wrap_HAL_UART_RxHalfCpltCallback
    HAL_UART_TxHalfCpltCallback=__wrap_HAL_UART_TxHalfCpltCallback
    HAL_UARTEx_RxEventCallback=__wrap_HAL_UARTEx_RxEventCallback
    HAL_UART_AbortCpltCallback=__wrap_HAL_UART_AbortCpltCallback
    HAL_UART_AbortTransmitCpltCallback=__wrap_HAL_UART_AbortTransmitCpltCallback
    HAL_UART_AbortReceiveCpltCallback=__wrap_HAL_UART_AbortReceiveCpltCallback
    HAL_SYSTICK_Callback=__wrap_HAL_SYSTICK_Callback
    HAL_TIM_OC_DelayElapsedCallback=__wrap_HAL_TIM_OC_DelayElapsedCallback
    HAL_TIM_PWM_PulseFinishedCallback=__wrap_HAL_TIM_PWM_PulseFinishedCallback
    HAL_TIM_IC_CaptureCallback=__wrap_HAL_TIM_IC_CaptureCallback
    HAL_TIM_TriggerCallback=__wrap_HAL_TIM_TriggerCallback
    HAL_TIM_PeriodElapsedHalfCpltCallback=__wrap_HAL_TIM_PeriodElapsedHalfCpltCallback
    HAL_TIM_PWM_PulseFinishedHalfCpltCallback=__wrap_HAL_TIM_PWM_PulseFinishedHalfCpltCallback

    # Nguồn, Xung nhịp & Flash Callbacks
    HAL_PWR_PVDCallback=__wrap_HAL_PWR_PVDCallback
    HAL_RCC_CSSCallback=__wrap_HAL_RCC_CSSCallback
    HAL_FLASH_EndOfOperationCallback=__wrap_HAL_FLASH_EndOfOperationCallback
    HAL_FLASH_OperationErrorCallback=__wrap_HAL_FLASH_OperationErrorCallback
)

# 2. Cấm GCC tự chèn hàm memset vào Bootloader (Sector 0)
set_source_files_properties(bootloader/sha/sha256.c PROPERTIES COMPILE_FLAGS "-fno-builtin")
