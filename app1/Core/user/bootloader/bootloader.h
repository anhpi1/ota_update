#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdint.h>
#include <stddef.h>
#include "stm32f4xx.h"
#include "sha/sha256.h"

#define ADDR_APP1_INFO 0x08004000
#define ADDR_APP2_INFO 0x08008000
#define ADDR_SERVICE_START 0x08060000
#define BOOT_FLASH_KEY1 0x45670123U
#define BOOT_FLASH_KEY2 0xCDEF89ABU

#define STATE_ERASED   0xFF
#define STATE_TESTING  0xFD  /* 0b11111101: App mới nạp, đang thử nghiệm */
#define STATE_STABLE   0xFC  /* 0b11111100: App đã xác nhận ổn định */
#define STATE_BAD      0x00  /* 0b00000000: App hỏng, cấm khởi động */

int bootloader(void);
int caculate_sha256(const BYTE *data, uint32_t len, const BYTE *hash_target);
void compute_sha256(const BYTE *data, uint32_t len, BYTE *hash_out);
int write_flash(uint32_t address, const BYTE *data, uint32_t len);
int prepare_ram_environment(uint32_t app_addr);
int boot_FlashUnlock(void);
void boot_FlashLock(void);
int boot_FlashWriteByte(uint32_t address, uint8_t data);

#endif