#include "bootloader.h"


extern uint32_t _flash_start;
extern uint32_t _flash_end;
extern uint32_t _metadata_start;
extern uint32_t _metadata_size;
extern uint32_t _slot1_start;
extern uint32_t _slot1_size;
extern uint32_t _slot2_start;
extern uint32_t _slot2_size;

extern uint32_t _s_ram_vector;
extern uint32_t _s_ram_app_vector;

int bootloader(void)
{
    __disable_irq();                                                                                            // Tắt toàn bộ ngắt để bàn giao sạch sẽ

    const BYTE bad_state = STATE_BAD;                                                                                           // 0x00: App hỏng, cấm khởi động
    const uint32_t slot1_start = (uint32_t)&_slot1_start;
    const uint32_t slot1_size  = (uint32_t)&_slot1_size;
    const uint32_t slot2_start = (uint32_t)&_slot2_start;
    const uint32_t slot2_size  = (uint32_t)&_slot2_size;

    const BYTE *hash_slot1 = (const BYTE *)(ADDR_APP1_INFO);                    // 32 byte tiếp theo là hash của image 
    const BYTE *boot_count_slot1 = (const BYTE *)(ADDR_APP1_INFO + 32);          // 1 byte tiếp theo là boot count của image
    const BYTE *state_slot1 = (const BYTE *)(ADDR_APP1_INFO + 33);               // 1 byte tiếp theo là state của image
    const uint32_t *img_size_slot1 = (const uint32_t *)(ADDR_APP1_INFO + 36);   // 4 byte kích thước thực của image

    const BYTE *hash_slot2 = (const BYTE *)(ADDR_APP2_INFO);                    // 32 byte tiếp theo là hash của image 
    const BYTE *boot_count_slot2 = (const BYTE *)(ADDR_APP2_INFO + 32);          // 1 byte tiếp theo là boot count của image
    const BYTE *state_slot2 = (const BYTE *)(ADDR_APP2_INFO + 33);               // 1 byte tiếp theo là state của image
    const uint32_t *img_size_slot2 = (const uint32_t *)(ADDR_APP2_INFO + 36);   // 4 byte kích thước thực của image

    uint32_t p_img = 0;                                                                                             //địa chỉ image sẽ nhảy tới

    //code xử lý roolback image, nếu image đang chạy bị lỗi, mặc dù sha đúng
    if (RCC->CSR & RCC_CSR_IWDGRSTF) { 
        RCC->CSR |= RCC_CSR_RMVF; // Xóa cờ reset phần cứng

        if (*state_slot1 == STATE_TESTING) {
            BYTE new_count = (*boot_count_slot1 << 1); // Hạ thêm 1 bit 1 -> 0
            write_flash((uint32_t)boot_count_slot1, &new_count, 1);
            if (new_count < 0xF8) { // Đã thử quá 3 lần mà vẫn crash!
                write_flash((uint32_t)state_slot1, &bad_state, 1); // Đánh dấu chết hẳn
            }
        } else if (*state_slot2 == STATE_TESTING) {
            BYTE new_count = (*boot_count_slot2 << 1);
            write_flash((uint32_t)boot_count_slot2, &new_count, 1);
            if (new_count < 0xF8) {
                write_flash((uint32_t)state_slot2, &bad_state, 1);
            }
        }
    }
    



    //STATE_TESTING là mã trạng thái báo img mới sẵn sàng
    uint32_t check_size_slot1 = (*img_size_slot1 > 0 && *img_size_slot1 <= slot1_size) ? *img_size_slot1 : slot1_size;
    uint32_t check_size_slot2 = (*img_size_slot2 > 0 && *img_size_slot2 <= slot2_size) ? *img_size_slot2 : slot2_size;

    if (*state_slot1==STATE_TESTING) p_img = slot1_start;
    else if (*state_slot2==STATE_TESTING) p_img = slot2_start;

    if(p_img == slot1_start) {
        if (caculate_sha256((const BYTE *)p_img, check_size_slot1, hash_slot1) != 0) 
        {
            write_flash((uint32_t)state_slot1, &bad_state, 1); // Lỗi tính toán hash, đánh dấu img slot1 là lỗi
            p_img = 0;
        }
    } else if(p_img == slot2_start) {
        if (caculate_sha256((const BYTE *)p_img, check_size_slot2, hash_slot2) != 0) 
        {
            write_flash((uint32_t)state_slot2, &bad_state, 1); // Lỗi tính toán hash, đánh dấu img slot2 là lỗi
            p_img = 0;
        }
    }

    // Nếu không có image nào sẵn sàng, chon img đang chạy ổn định (ưu tiên Slot 2 cho Node mồi)
    if(p_img == 0){
        if (*state_slot2==STATE_STABLE) p_img = slot2_start;
        else if (*state_slot1==STATE_STABLE) p_img = slot1_start;

         // nếu là app xuất xưởng (hash toàn 0xFF) thì tự tính sha và ghi vào metadata
        if(p_img == slot1_start) {
            if(hash_slot1[0] == 0xFF && hash_slot1[1] == 0xFF && hash_slot1[2] == 0xFF && hash_slot1[3] == 0xFF){
                BYTE calculated_hash[SHA256_BLOCK_SIZE];
                compute_sha256((const BYTE *)p_img, check_size_slot1, calculated_hash);
                if (write_flash((uint32_t)ADDR_APP1_INFO, calculated_hash, SHA256_BLOCK_SIZE) != 0) {
                    p_img = 0;
                }
            } else {
                if (caculate_sha256((const BYTE *)p_img, check_size_slot1, hash_slot1) != 0) 
                {
                    write_flash((uint32_t)state_slot1, &bad_state, 1); // Lỗi tính toán hash, đánh dấu img slot1 là lỗi
                    p_img = 0;
                }
            }
        } else if(p_img == slot2_start) {
            if(hash_slot2[0] == 0xFF && hash_slot2[1] == 0xFF && hash_slot2[2] == 0xFF && hash_slot2[3] == 0xFF){
                BYTE calculated_hash[SHA256_BLOCK_SIZE];
                compute_sha256((const BYTE *)p_img, check_size_slot2, calculated_hash);
                if (write_flash((uint32_t)ADDR_APP2_INFO, calculated_hash, SHA256_BLOCK_SIZE) != 0) {
                    p_img = 0;
                }
            } else {
                if (caculate_sha256((const BYTE *)p_img, check_size_slot2, hash_slot2) != 0) 
                {
                    write_flash((uint32_t)state_slot2, &bad_state, 1); // Lỗi tính toán hash, đánh dấu img slot2 là lỗi
                    p_img = 0;
                }
            }
        }
    }
   

    if(p_img == 0) while (1);

    // Chuẩn bị toàn bộ dữ liệu Service và Vector Table trên RAM
    if (prepare_ram_environment(p_img) != 0)
    {
        while (1); // Lỗi xác thực Service hoặc App magic -> Cấm nhảy, dừng ở Bootloader an toàn
    }

    // Kích hoạt bảng ngắt trên RAM (0x20000000)
    SCB->VTOR = (uint32_t)&_s_ram_vector;
    __DSB();
    __ISB();

    // Lấy địa chỉ Reset_Handler và nhảy thẳng sang App
    uint32_t app_reset_handler_addr = *(volatile uint32_t *)(p_img + 4);
    void (*app_reset_handler)(void) = (void (*)(void))app_reset_handler_addr;
    app_reset_handler();

    while (1); // Nếu App return, dừng lại ở đây
    
}

void compute_sha256(const BYTE *data, uint32_t len, BYTE *hash_out) {
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, hash_out);
}

int caculate_sha256(const BYTE *data, uint32_t len, const BYTE *hash_target) {
    BYTE hash[SHA256_BLOCK_SIZE];
    compute_sha256(data, len, hash);

    for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
        if (hash[i] != hash_target[i]) {
            return -1; // Hash không khớp
        }
    }
    return 0;
}

int write_flash(uint32_t address, const BYTE *data, uint32_t len) 
{
    // 1. Kiểm tra phạm vi Flash và tham số hợp lệ
    if (address < (uint32_t)&_flash_start || (address + len) > (uint32_t)&_flash_end || data == NULL || len == 0) {
        return -1; 
    }

    // 2. Mở khóa Flash bằng thanh ghi thuần
    if (boot_FlashUnlock() != 0) {
        return -2;
    }

    // 3. XÓA SẠCH CÁC CỜ LỖI TỒN ĐỌNG (ghi 1 vào các bit trong FLASH->SR để xóa)
    FLASH->SR = (FLASH_SR_EOP   | FLASH_SR_OPERR | 
                 FLASH_SR_WRPERR| FLASH_SR_PGAERR| 
                 FLASH_SR_PGPERR| FLASH_SR_PGSERR);

    // 4. Ghi từng byte trực tiếp qua thanh ghi
    for (size_t i = 0; i < len; i++) {
        if (boot_FlashWriteByte(address + i, data[i]) != 0) {
            boot_FlashLock(); // Khóa lại trước khi thoát nếu gặp lỗi
            return -3;        // Lỗi ghi phần cứng
        }
    }

    // 5. Khóa lại Flash để bảo vệ
    boot_FlashLock();

    return 0; // Thành công 100%
}



int boot_FlashUnlock(void)
{
    /* Nếu Flash đang bị khóa (bit LOCK = 1) */
    if (FLASH->CR & FLASH_CR_LOCK) 
    {
        /* Ghi chuỗi khóa theo đúng quy chuẩn phần cứng */
        FLASH->KEYR = BOOT_FLASH_KEY1;
        FLASH->KEYR = BOOT_FLASH_KEY2;

        /* Kiểm tra lại: nếu bit LOCK vẫn bằng 1 tức là mở khóa thất bại */
        if (FLASH->CR & FLASH_CR_LOCK) 
        {
            return -1; // Lỗi mở khóa
        }
    }

    return 0; // Đã mở khóa thành công
}

void boot_FlashLock(void) 
{
  /* Set the LOCK Bit to lock the FLASH Registers access */
  FLASH->CR |= FLASH_CR_LOCK;

}

int boot_FlashWriteByte(uint32_t address, uint8_t data)
{
    /* 1. Chờ Flash rảnh (bit BSY trong FLASH->SR về 0) */
    while (FLASH->SR & FLASH_SR_BSY);

    /* 2. Cấu hình kích thước ghi 1 byte (x8: PSIZE = 00) và kích hoạt chế độ ghi (PG = 1) */
    FLASH->CR &= ~FLASH_CR_PSIZE; // 8-bit parallelism
    FLASH->CR |= FLASH_CR_PG;

    /* 3. Ghi trực tiếp 1 byte vào ô nhớ Flash */
    *(volatile uint8_t *)address = data;

    /* 4. Rào cản bộ nhớ đảm bảo lệnh ghi hoàn tất */
    __DSB();

    /* 5. Chờ quá trình nạp vật lý hoàn tất */
    while (FLASH->SR & FLASH_SR_BSY);

    /* 6. Tắt bit lập trình (PG = 0) */
    FLASH->CR &= ~FLASH_CR_PG;

    /* 7. Kiểm tra các cờ lỗi phần cứng trong thanh ghi SR */
    if (FLASH->SR & (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR))
    {
        /* Xóa cờ lỗi bằng cách ghi 1 vào bit tương ứng */
        FLASH->SR = (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR);
        return -1; // Ghi lỗi
    }

    return 0; // Thành công
}

int prepare_ram_environment(uint32_t app_addr)
{
    const uint32_t *serv_desc = (const uint32_t *)ADDR_SERVICE_START;

    /* 1. Kiểm tra magic "SERV" (Word 5) để chắc chắn Service hợp lệ */
    if (serv_desc[5] != 0x53455256) 
    {
        return -1; // Lỗi: Service Sector 7 bị hỏng hoặc chưa nạp
    }

    /* Copy .service_data từ Flash Service sang RAM_SERVICE */
    uint32_t *p_src_data = (uint32_t *)serv_desc[0]; // _si_service_data
    uint32_t *p_dst_data = (uint32_t *)serv_desc[1]; // _s_service_data
    uint32_t *p_end_data = (uint32_t *)serv_desc[2]; // _e_service_data
    while (p_dst_data < p_end_data) {
        *p_dst_data++ = *p_src_data++;
    }

    /* Xóa 0 toàn bộ vùng .service_bss trên RAM_SERVICE */
    uint32_t *p_dst_bss = (uint32_t *)serv_desc[3];  // _s_service_bss
    uint32_t *p_end_bss = (uint32_t *)serv_desc[4];  // _e_service_bss
    while (p_dst_bss < p_end_bss) {
        *p_dst_bss++ = 0;
    }

    /* 2. Bốc đủ 128 vector (512 bytes) từ Flash của App lên RAM_VECTOR (0x20000000) */
    uint32_t *p_src_vec = (uint32_t *)app_addr;
    uint32_t *p_dst_vec = (uint32_t *)&_s_ram_vector;
    for (int i = 0; i < 128; i++) {
        p_dst_vec[i] = p_src_vec[i];
    }

    /* 3. Bốc 128 words (512 bytes) bảng hàm g_appVectors từ Flash App lên RAM_ROUTER */
    const uint32_t *app_desc = (const uint32_t *)(app_addr + 0x200);

    /* Kiểm tra magic "APP1" (0x41505031) tại Word 0 */
    if (app_desc[0] != 0x41505031)
    {
        return -2; // Lỗi: Phân vùng App hỏng hoặc không có bảng vector g_appVectors hợp lệ
    }

    uint32_t *p_dst_app = (uint32_t *)&_s_ram_app_vector;
    uint32_t words_to_copy = app_desc[2]; // Word 2 lưu số entry (mặc định 128)
    if (words_to_copy > 128) words_to_copy = 128;

    for (uint32_t i = 0; i < words_to_copy; i++) {
        p_dst_app[i] = app_desc[i];
    }

    return 0; // Thành công
}