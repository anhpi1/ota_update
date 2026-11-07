#include "serbackup.h"


static uint32_t serbackup_adrr_table[SERBACKUP_MAX_INDEX_TABLE] = {0};
static uint8_t serbackup_num_table_index = 0;
static uint32_t serbackup_last_backup = 0;
static osMutexId_t serbackup_Mutex = NULL;

serbackup_Status serbackup_Init(void)
{
    serbackup_Status status = serbackup_OK;
    if (drvflash_CheckHealth() != drvflash_OK) return serbackup_ERROR;
    if (drvcrc_CheckHealth() != drvcrc_OK) return serbackup_ERROR;
    serbackup_Mutex = osMutexNew(NULL);
    if (serbackup_Mutex == NULL) return serbackup_ERROR;

    //tìm bản ghi cuối cùng, thuật toán tìm kiếm nhị phân
    
    uint32_t addr_left = SERBACKUP_ADDR_SECTOR;
    uint32_t addr_right = SERBACKUP_ADDR_SECTOR + SERBACKUP_SIZE_SECTOR - 1;
    uint32_t addr = addr_left + (addr_right - addr_left) / 2;
    uint32_t cua_so_quet = (4 + SERBACKUP_MAX_DATA_LENGTH*4 + 4) + 8 ; // đơn vị bytes
    uint8_t is_find = 0;
    uint8_t found_anys = 0;

    uint32_t temp_record[1 + SERBACKUP_MAX_DATA_LENGTH + 1];    // bộ nhớ đệm kiểm tra dữ liệu không ép 
    serbackup_Header temp_header;                                                            // kiểu trực tiếp vì nguy cơ lệch địa chỉ 32 bit

    for (uint32_t i = 0; i < SERBACKUP_MAX_STEP ; i++)
    {
        for(uint32_t j=0; j <= cua_so_quet; j++) // đơn vị data header + max data +crc + dự phòng
        {
            if ((addr + j) > (SERBACKUP_ADDR_SECTOR + SERBACKUP_SIZE_SECTOR - 1)) break; // tránh đọc sang sector khác


            if (*(uint8_t*)(addr + j) == SERBACKUP_MAGIC_START) 
            {
                uint32_t crc = 0;
                memcpy(&temp_header, (void*)(addr + j), sizeof(serbackup_Header));
                serbackup_Header *header = &temp_header;
                if (header->length > SERBACKUP_MAX_DATA_LENGTH) continue; // Phải chặn ở đây trước!
                
                memcpy(temp_record, (void*)(addr + j), sizeof(serbackup_Header) + header->length*4 + 4);
                uint32_t *payload = temp_record + 1; 
                uint32_t *crc_ptr = temp_record + 1 + header->length; // con trỏ đến CRC trong bộ nhớ đệm   
                
                if(drvcrc_CaculateCRC(payload, header->length, &crc, 1000) != drvcrc_OK) 
                {
                    status = serbackup_BUSY;
                    is_find = 0;
                    goto end;
                }

                if (crc != *crc_ptr) continue;

                addr_left = addr + j ;
                addr = addr_left + (addr_right - addr_left) / 2;
                is_find = 1;
                found_anys = 1;
                break;
            }
        }

        if (!is_find) 
        {
            addr_right = addr;
            addr = addr_left + (addr_right - addr_left) / 2;
        }

        if ((addr_right - addr_left) <= cua_so_quet) break;
        is_find = 0;
    }
    
end:

    if(status == serbackup_BUSY) 
    {
        if (drvcrc_CheckHealth() != drvcrc_OK) status = serbackup_ERROR;
    }

    if(found_anys) 
    {
        serbackup_last_backup = addr_left;

        // Cập nhật con trỏ ghi flash sau bản ghi cuối cùng
        uint16_t length_index = 0;
        memcpy(&length_index, (const void *)(addr_left + 2), sizeof(uint16_t));; // Lấy độ dài payload từ header
        drvflash_SetWritePtr(SERBACKUP_NAME_SECTOR, addr_left + 4 + length_index*4 + 4); 

        // cập nhật bảng index với các bản ghi đã tìm thấy

        uint32_t cur = SERBACKUP_ADDR_SECTOR;
        while (cur <= serbackup_last_backup)
        {
            serbackup_Header hdr;
            memcpy(&hdr, (const void *)cur, sizeof(serbackup_Header));
            
            
            // Nếu header hỏng -> Đánh dấu lỗi và thoát ngay để xuống dưới Erase
            if (hdr.magic != SERBACKUP_MAGIC_START || hdr.length > SERBACKUP_MAX_DATA_LENGTH)
            {
                status = serbackup_BUSY;
                break;
            }

            // Cập nhật/ghi đè ID vào bảng
            for (uint8_t i = 0; i < SERBACKUP_MAX_INDEX_TABLE; i++)
            {
                if (serbackup_adrr_table[i] == 0)
                {
                    serbackup_adrr_table[i] = cur;
                    serbackup_num_table_index++;
                    break;
                }
                else if (*(uint8_t *)(serbackup_adrr_table[i] + 1) == hdr.id_data)
                {
                    serbackup_adrr_table[i] = cur; // Ghi đè địa chỉ mới hơn của ID này
                    break;
                }
            }

            cur += 4 + hdr.length * 4 + 4; // Nhảy cóc sang bản ghi tiếp theo
        }
    } 
    else status = serbackup_BUSY;  // (Không tìm thấy bản ghi nào trong cả Sector)
        

    // Chỉ khi không tìm thấy bản ghi nào HOẶC dữ liệu bị hỏng (status == serbackup_BUSY) mới Erase!
    if (status == serbackup_BUSY) 
    {
        drvflash_Erase(SERBACKUP_NAME_SECTOR, 1000);
        serbackup_last_backup = 0;
        serbackup_num_table_index = 0;
        status = serbackup_OK;
    }

    if(serbackup_OK != serbackup_CheckHealth()) status = serbackup_ERROR;
    if(status != serbackup_OK) serbackup_DeInit();


    return status;

}



serbackup_Status serbackup_SaveCheckpoint(uint8_t id_data, uint32_t *data, uint32_t length_index, uint32_t timeout)
{
    serbackup_Status status = serbackup_OK;
    if (data == NULL || length_index == 0 || length_index > SERBACKUP_MAX_DATA_LENGTH) return serbackup_ERROR;
    if(serbackup_CheckHealth() != serbackup_OK) return serbackup_ERROR;
    if(osMutexAcquire(serbackup_Mutex, timeout) != osOK) return serbackup_BUSY;

    uint8_t is_old_id = 0;
    uint8_t i = 0;
    for(; i < SERBACKUP_MAX_INDEX_TABLE; i++)
    {   
        if (serbackup_adrr_table[i] == 0) break;
        else if(*(uint8_t*)(serbackup_adrr_table[i] + 1) == id_data) 
        {
            is_old_id = 1;
            break; // đã có bản ghi với id_data này, ghi đè lên bản ghi cũ
        }
    }

    if (i >= SERBACKUP_MAX_INDEX_TABLE) // nếu duyệt hết không còn slot trống để lưu trữ bản ghi mới, báo lỗi
    {
        status = serbackup_FULL_ID; // Không còn chỗ lưu trữ bản ghi mới
        goto end;
    }

    serbackup_Header header;
    header.magic = SERBACKUP_MAGIC_START;
    header.id_data = id_data;
    header.length = length_index;

    uint32_t crc = 0;
    if(drvcrc_CaculateCRC(data, length_index, &crc, timeout) != drvcrc_OK) 
    {
        status = serbackup_BUSY;
        goto end;
    }

    uint8_t bufff[4+ SERBACKUP_MAX_DATA_LENGTH*4 + 4] = {0};
    memcpy(bufff, &header, 4);
    memcpy(bufff + 4 , data, length_index*4);
    memcpy(bufff + 4 + length_index*4, &crc, 4);

    if(drvflash_Write(SERBACKUP_NAME_SECTOR, bufff, 4 + length_index*4 + 4, timeout) != drvflash_OK)
    {
        status = serbackup_BUSY;
        goto end;
    }
    
end:

    if (status == serbackup_OK)
    {
        drvflash_SectorInfo info;
        drvflash_GetSectorInfo(SERBACKUP_NAME_SECTOR, &info);
        serbackup_adrr_table[i] = info.write_ptr - (4 + length_index * 4 + 4);
        serbackup_last_backup = serbackup_adrr_table[i];
        if (is_old_id == 0) serbackup_num_table_index++;
    }

    osMutexRelease(serbackup_Mutex);
    return status;
}


serbackup_Status serbackup_LoadCheckpoint(uint8_t id_data, uint32_t *data, uint32_t length_sindex, uint32_t timeout)
{
    serbackup_Status status = serbackup_OK;
    if (data == NULL || length_sindex == 0 || length_sindex > SERBACKUP_MAX_DATA_LENGTH) return serbackup_ERROR;
    if (serbackup_CheckHealth() != serbackup_OK) return serbackup_ERROR;
    if (osMutexAcquire(serbackup_Mutex, timeout) != osOK) return serbackup_BUSY;

    uint32_t addr = 0;

    // tìm địa chỉ id trong bảng chỉ mục
    for (uint8_t i = 0; i < SERBACKUP_MAX_INDEX_TABLE; i++)
    {
        if (serbackup_adrr_table[i] == 0) break;
        if (*(uint8_t *)(serbackup_adrr_table[i] + 1) == id_data)
        {
            addr = serbackup_adrr_table[i];
            break;
        }

    }

    if (addr == 0)
    {
        status = serbackup_ERROR; // Không tìm thấy ID này trong backup
        goto end;
    }


    serbackup_Header header;
    memcpy(&header, (const void *)addr, sizeof(serbackup_Header));
    if (header.magic != SERBACKUP_MAGIC_START || header.length != length_sindex)
    {
        status = serbackup_ERROR;
        goto end;
    }
    uint32_t *payload = (uint32_t *)(addr + 4);
    memcpy(data, payload, length_sindex * 4);

    uint32_t crc_calc = 0;
    uint32_t flash_crc = 0;
    memcpy(&flash_crc, (const void *)(addr + 4 + length_sindex * 4), sizeof(uint32_t));


    // Kiểm tra CRC để đảm bảo dữ liệu không bị hỏng
    if (drvcrc_CaculateCRC(data, length_sindex, &crc_calc, timeout) != drvcrc_OK || crc_calc != flash_crc)
    {
        status = serbackup_ERROR; // CRC không khớp hoặc lỗi driver ngoại vi
        goto end;
    }

end:
    osMutexRelease(serbackup_Mutex);
    return status;
}

serbackup_Status serbackup_DeInit(void)
{
    serbackup_Status status = serbackup_OK;
    if (serbackup_Mutex != NULL)
    {
        if (osMutexDelete(serbackup_Mutex) != osOK) status = serbackup_ERROR;
        serbackup_Mutex = NULL;
    }
    memset(serbackup_adrr_table, 0, sizeof(serbackup_adrr_table));
    serbackup_num_table_index = 0;
    serbackup_last_backup = 0;

    return status;
}

serbackup_Status serbackup_CheckHealth(void)
{
    if (serbackup_Mutex == NULL) return serbackup_ERROR;
    if (drvflash_CheckHealth() != drvflash_OK) return serbackup_ERROR;
    if (drvcrc_CheckHealth() != drvcrc_OK) return serbackup_ERROR;

    return serbackup_OK;
}