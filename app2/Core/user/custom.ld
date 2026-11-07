/*

tìm đến đòng sau trong <Thư mục dự án>/cmake/gcc-arm-none-eabi.cmake
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${CMAKE_SOURCE_DIR}/STM32F411xx_FLASH.ld\"")
sửa thành 
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${CMAKE_SOURCE_DIR}/Core/user/custom.ld\"")

*/

/* Entry Point */
ENTRY(Boot_Reset_Handler)

/* Ép Linker LUÔN trích xuất (từ libg_nano.a / libgcc.a) mọi module .o mà Service đang ghim ở Sector 7,
 * bất kể code App có gọi hay không. KEEP() chỉ giữ section của file ĐÃ được trích xuất,
 * không tự rút file .o ra khỏi thư viện .a -> cần EXTERN() để tránh co/giãn .service_text khi đổi App.
 * (Mỗi symbol dưới đây đại diện cho 1 module .o, thứ tự theo bảng "Archive member included" trong .map) */
EXTERN(
    __aeabi_uldivmod __udivmoddi4 __aeabi_ldiv0
    atoi _strtol_r _ctype_ strlen strcmp strtok __strtok_r
    memcpy memset memcmp memmove memchr
    printf puts fiprintf _vfprintf_r _printf_i __sfvwrite_r
    __swbuf_r __swsetup_r _fflush_r __stdio_exit_handler _fwalk_sglue __sread __smakebuf_r
    malloc _malloc_r _free_r _realloc_r _malloc_usable_size_r __malloc_lock
    __retarget_lock_init_recursive
    _sbrk_r _read_r _write_r _close_r _lseek_r _fstat_r _isatty_r
    errno _impure_ptr __errno abort __assert_func raise _kill_r exit
)

/* Specify the memory areas */
MEMORY
{
  RAM_VECTOR        (xrw) : ORIGIN = 0x20000000, LENGTH = 512 + 512 /* Bảng ngắt RAM + App Router Table (128 words) */
  RAM_SERVICE       (xrw) : ORIGIN = 0x20000400, LENGTH = 64K - 1024 /* Vùng nhớ bất khả xâm phạm của Service */
  RAM_APP           (xrw) : ORIGIN = 0x20010000, LENGTH = 64K       /* Vùng nhớ của User App */

  FLASH_BOOTLOADER : ORIGIN = 0x08000000, LENGTH = 16K
  FLASH_META_SLOT1 : ORIGIN = 0x08004000, LENGTH = 16K
  FLASH_META_SLOT2 : ORIGIN = 0x08008000, LENGTH = 16K
  FLASH_CHECK_POINT : ORIGIN = 0x0800C000, LENGTH = 16K
  FLASH_DIFF_PACK : ORIGIN = 0x08010000, LENGTH = 64K
  FLASH_APP1 : ORIGIN = 0x08020000, LENGTH = 128K
  FLASH_APP2 : ORIGIN = 0x08040000, LENGTH = 128K
  FLASH_SERVICE : ORIGIN = 0x08060000, LENGTH = 128K
}

/* Chọn Slot nạp cho App: đổi FLASH_APP1 <-> FLASH_APP2 tại đây */
REGION_ALIAS("FLASH_APP", FLASH_APP2);

_flash_start      = ORIGIN(FLASH_BOOTLOADER);
_flash_end        = ORIGIN(FLASH_SERVICE) + LENGTH(FLASH_SERVICE);
_meta_slot1_start = ORIGIN(FLASH_META_SLOT1);
_meta_slot1_size  = LENGTH(FLASH_META_SLOT1);
_meta_slot2_start = ORIGIN(FLASH_META_SLOT2);
_meta_slot2_size  = LENGTH(FLASH_META_SLOT2);
_slot1_start      = ORIGIN(FLASH_APP1);
_slot1_size       = LENGTH(FLASH_APP1);
_slot2_start      = ORIGIN(FLASH_APP2);
_slot2_size       = LENGTH(FLASH_APP2);

/* Flash Geometry Exported Symbols for C */
/* Highest address of the user mode stack */
_estack = ORIGIN(RAM_APP) + LENGTH(RAM_APP);    /* Đỉnh RAM (0x20020000) */    /* end of RAM */
_sstack = _estack - _Min_Stack_Size;
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x200;      /* required amount of heap  */
_Min_Stack_Size = 0x400; /* required amount of stack */

/* Define output sections */
SECTIONS
{
  /*/////////////////////////////////////// bootloader ///////////////////////////////////////*/

  .boot_vector :
  {
    . = ALIGN(4);
    KEEP(*(.boot_vector))
    . = ALIGN(4);
  } >FLASH_BOOTLOADER

  .bootloader_text :
  {
    . = ALIGN(4);
    KEEP(*bootloader*(.text .text* .rodata .rodata*))
    KEEP(*sha256*(.text .text* .rodata .rodata*))
    . = ALIGN(4);
  } >FLASH_BOOTLOADER


  /*/////////////////////////////////////// SERVICE (Sector 7) ///////////////////////////////////////*/
  .service_text :
  {
    . = ALIGN(4);
    
    KEEP(*(.service_vector)) 

    /* 1. Thư viện Service & Driver người dùng tự viết (đổi từ user_lib sang service_lib) */
    KEEP(*service_lib*(.text .text* .rodata .rodata*))
    KEEP(*hook_lib*(.text .text* .rodata .rodata*))

    /* 2. Giữ toàn bộ hàm của 17 file HAL Driver cốt lõi trên Service */
    KEEP(*stm32f4xx_hal.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_cortex.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_rcc.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_rcc_ex.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_gpio.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_dma.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_dma_ex.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_uart.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_flash.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_flash_ex.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_flash_ramfunc.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_crc.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_pwr.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_pwr_ex.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_tim.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_tim_ex.c.obj(.text .text* .rodata .rodata*))
    KEEP(*stm32f4xx_hal_exti.c.obj(.text .text* .rodata .rodata*))

    /* 3. Toàn bộ Kernel FreeRTOS */
    KEEP(*FreeRTOS*(.text .text* .rodata .rodata*))

    /* 5. Runtime Library Helpers (libc_nano, libg_nano, libgcc, libm) 
     * Gom toàn bộ các hàm runtime hiện tại đang có địa chỉ trong file .map lên Service
     */
    /* Nhóm toán học chia 64-bit & bẫy chia 0 (libgcc.a) */
    KEEP(*libgcc.a:_aeabi_uldivmod.o(.text .text*))
    KEEP(*libgcc.a:_udivmoddi4.o(.text .text*))
    KEEP(*libgcc.a:_dvmd_tls.o(.text .text*))

    /* Nhóm thao tác chuỗi & bộ nhớ (libc_nano / libg_nano) */
    KEEP(*libc_a-memcpy-stub.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-memset.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-memcmp.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-memmove.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-memchr.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-strcmp.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-strlen.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-strtok.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-strtok_r.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-atoi.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-strtol.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-ctype_.o(.text .text* .rodata .rodata*))

    /* Nhóm in ấn & định dạng Stdio Engine */
    KEEP(*libc_a-printf.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-puts.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-fprintf.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-nano-vfprintf.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-nano-vfprintf_i.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-fvwrite.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-wbuf.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-wsetup.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-fflush.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-findfp.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-fwalk.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-stdio.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-makebuf.o(.text .text* .rodata .rodata*))

    /* Nhóm quản lý Heap & System & Locks */
    KEEP(*libc_a-malloc.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-mallocr.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-freer.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-reallocr.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-msizer.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-mlock.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-lock.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-sbrkr.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-readr.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-writer.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-closer.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-lseekr.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-fstatr.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-isattyr.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-errno.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-abort.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-assert.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-signal.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-signalr.o(.text .text* .rodata .rodata*))
    KEEP(*libc_a-exit.o(.text .text* .rodata .rodata*))

    . = ALIGN(4);
  } >FLASH_SERVICE

    /* Dữ liệu có khởi tạo của Service */
  .service_data :
  {
    . = ALIGN(4);
    _s_service_data = .;

    /* 1. Thư viện Service & Driver người dùng tự viết (đổi từ user_lib sang service_lib) */
    KEEP(*service_lib*(.data .data* .RamFunc .RamFunc*))
    KEEP(*hook_lib*(.data .data* .RamFunc .RamFunc*))

    /* 2. Biến dữ liệu của 17 file HAL Driver cốt lõi trên Service */
    KEEP(*stm32f4xx_hal.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_cortex.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_rcc.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_rcc_ex.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_gpio.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_dma.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_dma_ex.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_uart.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_flash.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_flash_ex.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_flash_ramfunc.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_crc.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_pwr.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_pwr_ex.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_tim.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_tim_ex.c.obj(.data .data*))
    KEEP(*stm32f4xx_hal_exti.c.obj(.data .data*))

    /* 3. Toàn bộ Kernel FreeRTOS */
    KEEP(*FreeRTOS*(.data .data*))

    /* 5. Biến toàn cục có khởi tạo của các module runtime trên */
    KEEP(*libgcc.a:_aeabi_uldivmod.o(.data .data*))
    KEEP(*libgcc.a:_udivmoddi4.o(.data .data*))
    KEEP(*libgcc.a:_dvmd_tls.o(.data .data*))
    KEEP(*libc_a-memcpy-stub.o(.data .data*))
    KEEP(*libc_a-memset.o(.data .data*))
    KEEP(*libc_a-memcmp.o(.data .data*))
    KEEP(*libc_a-memmove.o(.data .data*))
    KEEP(*libc_a-memchr.o(.data .data*))
    KEEP(*libc_a-strcmp.o(.data .data*))
    KEEP(*libc_a-strlen.o(.data .data*))
    KEEP(*libc_a-strtok.o(.data .data*))
    KEEP(*libc_a-strtok_r.o(.data .data*))
    KEEP(*libc_a-atoi.o(.data .data*))
    KEEP(*libc_a-strtol.o(.data .data*))
    KEEP(*libc_a-ctype_.o(.data .data*))
    KEEP(*libc_a-printf.o(.data .data*))
    KEEP(*libc_a-puts.o(.data .data*))
    KEEP(*libc_a-fprintf.o(.data .data*))
    KEEP(*libc_a-nano-vfprintf.o(.data .data*))
    KEEP(*libc_a-nano-vfprintf_i.o(.data .data*))
    KEEP(*libc_a-fvwrite.o(.data .data*))
    KEEP(*libc_a-wbuf.o(.data .data*))
    KEEP(*libc_a-wsetup.o(.data .data*))
    KEEP(*libc_a-fflush.o(.data .data*))
    KEEP(*libc_a-findfp.o(.data .data*))
    KEEP(*libc_a-fwalk.o(.data .data*))
    KEEP(*libc_a-stdio.o(.data .data*))
    KEEP(*libc_a-makebuf.o(.data .data*))
    KEEP(*libc_a-malloc.o(.data .data*))
    KEEP(*libc_a-mallocr.o(.data .data*))
    KEEP(*libc_a-freer.o(.data .data*))
    KEEP(*libc_a-reallocr.o(.data .data*))
    KEEP(*libc_a-msizer.o(.data .data*))
    KEEP(*libc_a-mlock.o(.data .data*))
    KEEP(*libc_a-lock.o(.data .data*))
    KEEP(*libc_a-sbrkr.o(.data .data*))
    KEEP(*libc_a-readr.o(.data .data*))
    KEEP(*libc_a-writer.o(.data .data*))
    KEEP(*libc_a-closer.o(.data .data*))
    KEEP(*libc_a-lseekr.o(.data .data*))
    KEEP(*libc_a-fstatr.o(.data .data*))
    KEEP(*libc_a-isattyr.o(.data .data*))
    KEEP(*libc_a-reent.o(.data .data*))
    KEEP(*libc_a-impure.o(.data .data*))
    KEEP(*libc_a-errno.o(.data .data*))
    KEEP(*libc_a-abort.o(.data .data*))
    KEEP(*libc_a-assert.o(.data .data*))
    KEEP(*libc_a-signal.o(.data .data*))
    KEEP(*libc_a-signalr.o(.data .data*))
    KEEP(*libc_a-exit.o(.data .data*))

    . = ALIGN(4);
    _e_service_data = .;
  } >RAM_SERVICE AT> FLASH_SERVICE
  _si_service_data = LOADADDR(.service_data);

  /* Biến bằng 0 (.bss) của Service */
  .service_bss (NOLOAD) :
  {
    . = ALIGN(4);
    _s_service_bss = .;

    /* 1. Thư viện Service & Driver người dùng tự viết (đổi từ user_lib sang service_lib) */
    KEEP(*service_lib*(.bss .bss* COMMON))
    KEEP(*hook_lib*(.bss .bss* COMMON))

    /* 2. Biến bss của 17 file HAL Driver cốt lõi trên Service */
    KEEP(*stm32f4xx_hal.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_cortex.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_rcc.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_rcc_ex.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_gpio.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_dma.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_dma_ex.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_uart.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_flash.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_flash_ex.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_flash_ramfunc.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_crc.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_pwr.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_pwr_ex.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_tim.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_tim_ex.c.obj(.bss .bss* COMMON))
    KEEP(*stm32f4xx_hal_exti.c.obj(.bss .bss* COMMON))

    /* 3. Toàn bộ Kernel FreeRTOS */
    KEEP(*FreeRTOS*(.bss .bss* COMMON))
    
    /* 5. Biến toàn cục bằng 0 của các module runtime trên */
    KEEP(*libgcc.a:_aeabi_uldivmod.o(.bss .bss* COMMON))
    KEEP(*libgcc.a:_udivmoddi4.o(.bss .bss* COMMON))
    KEEP(*libgcc.a:_dvmd_tls.o(.bss .bss* COMMON))
    KEEP(*libc_a-memcpy-stub.o(.bss .bss* COMMON))
    KEEP(*libc_a-memset.o(.bss .bss* COMMON))
    KEEP(*libc_a-memcmp.o(.bss .bss* COMMON))
    KEEP(*libc_a-memmove.o(.bss .bss* COMMON))
    KEEP(*libc_a-memchr.o(.bss .bss* COMMON))
    KEEP(*libc_a-strcmp.o(.bss .bss* COMMON))
    KEEP(*libc_a-strlen.o(.bss .bss* COMMON))
    KEEP(*libc_a-strtok.o(.bss .bss* COMMON))
    KEEP(*libc_a-strtok_r.o(.bss .bss* COMMON))
    KEEP(*libc_a-atoi.o(.bss .bss* COMMON))
    KEEP(*libc_a-strtol.o(.bss .bss* COMMON))
    KEEP(*libc_a-ctype_.o(.bss .bss* COMMON))
    KEEP(*libc_a-printf.o(.bss .bss* COMMON))
    KEEP(*libc_a-puts.o(.bss .bss* COMMON))
    KEEP(*libc_a-fprintf.o(.bss .bss* COMMON))
    KEEP(*libc_a-nano-vfprintf.o(.bss .bss* COMMON))
    KEEP(*libc_a-nano-vfprintf_i.o(.bss .bss* COMMON))
    KEEP(*libc_a-fvwrite.o(.bss .bss* COMMON))
    KEEP(*libc_a-wbuf.o(.bss .bss* COMMON))
    KEEP(*libc_a-wsetup.o(.bss .bss* COMMON))
    KEEP(*libc_a-fflush.o(.bss .bss* COMMON))
    KEEP(*libc_a-findfp.o(.bss .bss* COMMON))
    KEEP(*libc_a-fwalk.o(.bss .bss* COMMON))
    KEEP(*libc_a-stdio.o(.bss .bss* COMMON))
    KEEP(*libc_a-makebuf.o(.bss .bss* COMMON))
    KEEP(*libc_a-malloc.o(.bss .bss* COMMON))
    KEEP(*libc_a-mallocr.o(.bss .bss* COMMON))
    KEEP(*libc_a-freer.o(.bss .bss* COMMON))
    KEEP(*libc_a-reallocr.o(.bss .bss* COMMON))
    KEEP(*libc_a-msizer.o(.bss .bss* COMMON))
    KEEP(*libc_a-mlock.o(.bss .bss* COMMON))
    KEEP(*libc_a-lock.o(.bss .bss* COMMON))
    KEEP(*libc_a-sbrkr.o(.bss .bss* COMMON))
    KEEP(*libc_a-readr.o(.bss .bss* COMMON))
    KEEP(*libc_a-writer.o(.bss .bss* COMMON))
    KEEP(*libc_a-closer.o(.bss .bss* COMMON))
    KEEP(*libc_a-lseekr.o(.bss .bss* COMMON))
    KEEP(*libc_a-fstatr.o(.bss .bss* COMMON))
    KEEP(*libc_a-isattyr.o(.bss .bss* COMMON))
    KEEP(*libc_a-reent.o(.bss .bss* COMMON))
    KEEP(*libc_a-impure.o(.bss .bss* COMMON))
    KEEP(*libc_a-errno.o(.bss .bss* COMMON))
    KEEP(*libc_a-abort.o(.bss .bss* COMMON))
    KEEP(*libc_a-assert.o(.bss .bss* COMMON))
    KEEP(*libc_a-signal.o(.bss .bss* COMMON))
    KEEP(*libc_a-signalr.o(.bss .bss* COMMON))
    KEEP(*libc_a-exit.o(.bss .bss* COMMON))

    . = ALIGN(4);
    _e_service_bss = .;
  } >RAM_SERVICE




  /*/////////////////////////////////////// app ///////////////////////////////////////*/
  /* The startup code goes first into FLASH */
    .isr_vector :
  {
    . = ALIGN(512);
    _s_ram_vector = .;         /* Tạo nhãn đầu RAM_VECTOR cho Bootloader dùng */
    KEEP(*(.isr_vector))
    . = ALIGN(512);
    _e_ram_vector = .;         
  } >RAM_VECTOR AT> FLASH_APP

  /* Bảng Vector Hàm của App: 128 words = 512 bytes (0x20000200 - 0x200003FF) */
  .app_vector :
  {
    . = ALIGN(256);
    _s_ram_app_vector = .;     /* Tạo nhãn RAM đầu bảng Router (0x20000200) */
    KEEP(*(.app_vector))
    . = ALIGN(512);
    _e_ram_app_vector = .;     /* Nhãn RAM kết thúc (0x20000400) */
  } >RAM_VECTOR AT> FLASH_APP


  .ARM.extab (READONLY) : /* The "READONLY" keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >FLASH_APP

  .ARM (READONLY) : /* The "READONLY" keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >FLASH_APP

   /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH_APP

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH_APP

  .preinit_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } >FLASH_APP

  .init_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } >FLASH_APP

  .fini_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } >FLASH_APP
  
  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */

    . = ALIGN(4);
  } >RAM_APP AT> FLASH_APP

 /* Initialized TLS data section */
  .tdata : ALIGN(4)
  {
    *(.tdata .tdata.* .gnu.linkonce.td.*)
    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
    PROVIDE(__data_end = .);
    PROVIDE(__tdata_end = .);
  } >RAM_APP AT> FLASH_APP

  PROVIDE( __tdata_start = ADDR(.tdata) );
  PROVIDE( __tdata_size = __tdata_end - __tdata_start );

  PROVIDE( __data_start = ADDR(.data) );
  PROVIDE( __data_size = __data_end - __data_start );

  PROVIDE( __tdata_source = LOADADDR(.tdata) );
  PROVIDE( __tdata_source_end = LOADADDR(.tdata) + SIZEOF(.tdata) );
  PROVIDE( __tdata_source_size = __tdata_source_end - __tdata_source );

  PROVIDE( __data_source = LOADADDR(.data) );
  PROVIDE( __data_source_end = __tdata_source_end );
  PROVIDE( __data_source_size = __data_source_end - __data_source );

  /* Kích thước thực tế của App trong Flash (căn chẵn 4-byte) */
  _app_size = ALIGN(__data_source_end - ORIGIN(FLASH_APP), 4);
  /* Uninitialized data section */
  .tbss (NOLOAD) : ALIGN(4)
  {
     /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.tbss .tbss.*)
    . = ALIGN(4);
    PROVIDE( __tbss_end = . );
  } >RAM_APP

  PROVIDE( __tbss_start = ADDR(.tbss) );
  PROVIDE( __tbss_size = __tbss_end - __tbss_start );
  PROVIDE( __tbss_offset = ADDR(.tbss) - ADDR(.tdata) );

  PROVIDE( __tls_base = __tdata_start );
  PROVIDE( __tls_end = __tbss_end );
  PROVIDE( __tls_size = __tls_end - __tls_base );
  PROVIDE( __tls_align = MAX(ALIGNOF(.tdata), ALIGNOF(.tbss)) );
  PROVIDE( __tls_size_align = (__tls_size + __tls_align - 1) & ~(__tls_align - 1) );
  PROVIDE( __arm32_tls_tcb_offset = MAX(8, __tls_align) );
  PROVIDE( __arm64_tls_tcb_offset = MAX(16, __tls_align) );

  .bss (NOLOAD) : ALIGN(4)
  {
    *(.bss)
    *(.bss*)
    *(COMMON)

      . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
      PROVIDE( __bss_end = .);
  } >RAM_APP
  PROVIDE( __non_tls_bss_start = ADDR(.bss) );

  PROVIDE( __bss_start = __tbss_start );
  PROVIDE( __bss_size = __bss_end - __bss_start );

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack (NOLOAD) :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM_APP



  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a:* ( * )
    libm.a:* ( * )
    libgcc.a:* ( * )
  }

    /*/////////////////////////////////////// metadata ///////////////////////////////////////*/

    .app1_metadata :
    {
      . = ALIGN(4);
      /* Slot 1: Metadata xuất xưởng (Sector 1) */
      FILL(0xFF);
      . = . + 32;           /* 32 byte hash = 0xFF (offset 0..31) */
      BYTE(0xFF);           /* boot_count = 0xFF (offset 32) */
      BYTE(0xFC);           /* state = 0xFC (offset 33) */
      . = . + 2;            /* 2 bytes padding (offset 34..35) */
      LONG(_app_size);      /* image_size 32-bit (offset 36..39) */
      . = ALIGN(4);
    } >FLASH_META_SLOT1

    .app2_metadata :
    {
      . = ALIGN(4);
      /* Slot 2: Metadata xuất xưởng (Sector 2) */
      FILL(0xFF);
      . = . + 32;           /* 32 byte hash = 0xFF (offset 0..31) */
      BYTE(0xFF);           /* boot_count = 0xFF (offset 32) */
      BYTE(0xFC);           /* state = 0xFC (offset 33) */
      . = . + 2;            /* 2 bytes padding (offset 34..35) */
      LONG(_app_size);      /* image_size 32-bit (offset 36..39) */
      . = ALIGN(4);
    } >FLASH_META_SLOT2


}

/* =========================================================================
 * TƯỜNG LỬA LINKER MỘT CHIỀU (ONE-WAY CROSS-REFERENCE FIREWALL)
 * ========================================================================= */
/* 1. Cấm Bootloader (Sector 0) chứa bất kỳ địa chỉ nào của Service hoặc App */
NOCROSSREFS_TO(.service_text .boot_vector .bootloader_text)
NOCROSSREFS_TO(.service_data .boot_vector .bootloader_text)
NOCROSSREFS_TO(.service_bss  .boot_vector .bootloader_text)
NOCROSSREFS_TO(.text         .boot_vector .bootloader_text)
NOCROSSREFS_TO(.rodata       .boot_vector .bootloader_text)
NOCROSSREFS_TO(.data         .boot_vector .bootloader_text)
NOCROSSREFS_TO(.bss          .boot_vector .bootloader_text)

/* 2. Cấm Service (Sector 7 & RAM_SERVICE) chứa bất kỳ địa chỉ trực tiếp nào của App */
NOCROSSREFS_TO(.text         .service_text .service_data .service_bss)
NOCROSSREFS_TO(.rodata       .service_text .service_data .service_bss)
NOCROSSREFS_TO(.data         .service_text .service_data .service_bss)
NOCROSSREFS_TO(.bss          .service_text .service_data .service_bss)
