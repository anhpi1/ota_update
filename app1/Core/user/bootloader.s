.syntax unified
.cpu cortex-m4
.thumb

.section .boot_vector, "a"

.global g_bootVectors
    g_bootVectors:
        .word _estack
        .word Boot_Reset_Handler

/* Bảng mô tả 10 words cố định tại đầu Service */
.section .service_vector, "a"
.global g_serviceVectors
    g_serviceVectors:
        .word _si_service_data    /* Word 0: LMA nguồn Flash của data */
        .word _s_service_data     /* Word 1: VMA đích RAM bắt đầu data */
        .word _e_service_data     /* Word 2: VMA đích RAM kết thúc data */
        .word _s_service_bss      /* Word 3: RAM bắt đầu bss */
        .word _e_service_bss      /* Word 4: RAM kết thúc bss */
        .word 0x53455256          /* Word 5: Magic "SERV" */
        .word 0                   /* Word 6: Dự phòng */
        .word 0                   /* Word 7: Dự phòng */
        .word 0                   /* Word 8: Dự phòng */
        .word 0                   /* Word 9: Dự phòng */

.section .text
.global Boot_Reset_Handler

.type Boot_Reset_Handler, %function
    Boot_Reset_Handler:
        bl bootloader
        b  .