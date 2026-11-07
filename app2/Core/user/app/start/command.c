#include "stdlib.h"
#include "string.h"
#include "driver/uart/drvuart.h"
#include "driver/flash/drvflash.h"
#include "service/cli/serCli.h"
#include "service/logging/serLog.h"
#include "service/ota/serota.h"


extern drvuart_Handle uartHandleDebug; // Declare the debug UART handle as external
extern drvuart_Handle uartHandleOTA; // Declare the OTA UART handle as external

sercli_Status cmdHelp(const uint8_t argc, const char * const argv[]);
sercli_Status cmdTest(const uint8_t argc, const char * const argv[]);

sercli_Status cmdFlashErase(const uint8_t argc, const char * const argv[]);
sercli_Status cmdFlashWrite(const uint8_t argc, const char * const argv[]);
sercli_Status cmdFlashGetInfo(const uint8_t argc, const char * const argv[]);
sercli_Status cmdOtaStart(const uint8_t argc, const char * const argv[]);
sercli_Status cmdHello(const uint8_t argc, const char * const argv[]);

const sercli_CommandStruct command_list[] = {
    {"help", cmdHelp, "Display this help message"},
    {"test", cmdTest, "Test command for demonstration"},
    {"flash_erase", cmdFlashErase, "Erase a specific flash sector. Usage: flash_erase <sector_number>"},
    {"flash_write", cmdFlashWrite, "Write data to a specific flash sector. Usage: flash_write <sector_number> <data>"},
    {"flash_get_info", cmdFlashGetInfo, "Get information about a specific flash sector. Usage: flash_get_info <sector_number>"},
    {"ota_start", cmdOtaStart, "Start OTA firmware transmission to peer node"},
    {"hello", cmdHello, "Print Hello World greeting from App 2"},
    {NULL, NULL, NULL} // Null-terminated list
};

sercli_Status cmdHelp(const uint8_t argc, const char * const argv[])
{
    printf("Available commands:\n");
    for (const sercli_CommandStruct *cmd = command_list; cmd->command != NULL; cmd++) {
        printf("  %s: %s\n", cmd->command, cmd->help);
    }
    return sercli_OK;
}

sercli_Status cmdTest(const uint8_t argc, const char * const argv[])
{
    if (drvuart_CheckHealth(&uartHandleDebug) == drvuart_OK) printf("OK   : Debug UART.\n");
    else printf("ERROR: Debug UART.\n");
    if (drvuart_CheckHealth(&uartHandleOTA) == drvuart_OK) printf("OK   : OTA UART.\n");
    else printf("ERROR: OTA UART.\n");
    if (serlog_CheckHealth() == serlog_OK) printf("OK   : Logging system.\n");
    else printf("ERROR: Logging system.\n");
    if (sercli_CheckHealth() == sercli_OK) printf("OK   : Command system.\n");
    else printf("ERROR: Command system.\n");
    if (drvflash_CheckHealth() == drvflash_OK) printf("OK   : Flash driver.\n");
    else printf("ERROR: Flash driver.\n");
    return sercli_OK;
}

sercli_Status cmdFlashErase(const uint8_t argc, const char * const argv[])
{
    if (argc != 2) {
        printf("Usage: flash_erase <sector_number>\n");
        return sercli_ERROR;
    }

    uint8_t sector = (uint8_t)atoi(argv[1]);
    drvflash_Status status = drvflash_Unlock(sector, osWaitForever);
    if (status != drvflash_OK) {
        printf("Failed to unlock sector %d. Error code: %d\n", sector, status);
        return sercli_ERROR;
    }
    status = drvflash_Erase(sector,osWaitForever);
    drvflash_Lock(sector); // Lock the sector if unlock failed

    if (status == drvflash_OK) {
        printf("Sector %d erased successfully.\n", sector);
        return sercli_OK;
    } else {
        printf("Failed to erase sector %d. Error code: %d\n", sector, status);
        return sercli_ERROR;
    }
}

sercli_Status cmdFlashWrite(const uint8_t argc, const char * const argv[])
{
    if (argc != 3) {
        printf("Usage: flash_write <sector_number> <data>\n");
        return sercli_ERROR;
    }

    uint8_t sector = (uint8_t)atoi(argv[1]);
    const char *data_str = argv[2];
    uint32_t data_length = strlen(data_str);

    drvflash_Status status = drvflash_Unlock(sector, osWaitForever);
    if (status != drvflash_OK) {
        printf("Failed to unlock sector %d. Error code: %d\n", sector, status);
        return sercli_ERROR;
    }
    
    status = drvflash_Write(sector, (uint8_t *)data_str, data_length, osWaitForever);
    drvflash_Lock(sector); 
    if (status == drvflash_OK) {
        printf("Data written to sector %d successfully.\n", sector);
        return sercli_OK;
    } else {
        printf("Failed to write data to sector %d. Error code: %d\n", sector, status);
        return sercli_ERROR;
    }
}

sercli_Status cmdFlashGetInfo(const uint8_t argc, const char * const argv[])
{
    if (argc != 2) {
        printf("Usage: flash_get_info <sector_number>\n");
        return sercli_ERROR;
    }

    uint8_t sector = (uint8_t)atoi(argv[1]);
    drvflash_SectorInfo info;
    drvflash_Status status = drvflash_GetSectorInfo(sector, &info);
    if (status == drvflash_OK) {
        printf("Sector %d Info:\n", sector);
        printf("  Address: 0x%08X\n", (unsigned int)info.address);
        printf("  Size: %lu bytes\n", (unsigned long)info.size);
        printf("  Current Data Address: 0x%08X\n", (unsigned int)info.write_ptr);
        return sercli_OK;
    } else {
        printf("Failed to get info for sector %d. Error code: %d\n", sector, status);
        return sercli_ERROR;
    }
}

sercli_Status cmdOtaStart(const uint8_t argc, const char * const argv[])
{
    (void)argc;
    (void)argv;
    serota_Packet tx_packet;
    serota_Packet rx_feedback;

    printf("[OTA MASTER] Bat dau phien OTA...\n");

    /* 1. Khoi tao phien Master va lay goi PREPARE_UPDATE dau tien */
    if (serota_MasterBegin(&tx_packet) != serota_OK) {
        printf("[OTA MASTER] Loi: Khoi tao Master that bai! (Kiem tra Metadata/Flash)\n");
        return sercli_ERROR;
    }

    printf("[OTA MASTER] Khoi tao thanh cong. Dang bat dau truyen...\n");

    /* 2. Vong lap Stop-and-Wait */
    serota_Status status = serota_BUSY;
    while (status == serota_BUSY)
    {
        /* Truyen goi tin qua UART2 DMA */
        if (drvuart_Transmit(&uartHandleOTA, (const uint8_t *)&tx_packet, sizeof(serota_Packet), 1000) != drvuart_OK) {
            printf("[OTA MASTER] Loi: Truyen UART that bai!\n");
            return sercli_ERROR;
        }

        /* Cho nhan phan hoi: Timeout 3000ms cho PREPARE (Flash Erase 128KB), 1000ms cho cac goi khac */
        uint16_t rx_len = 0;
        uint32_t wait_ms = (tx_packet.header.type == serota_MSG_PREPARE_UPDATE) ? 3000 : 1000;
        drvuart_Status u_stat = drvuart_Receive(&uartHandleOTA, (uint8_t *)&rx_feedback, &rx_len, wait_ms);

        const serota_Packet *p_fb = (u_stat == drvuart_OK && rx_len == sizeof(serota_Packet)) ? &rx_feedback : NULL;

        /* Nap feedback vao FSM Master de lay goi tiep theo */
        status = serota_ProcessPacketMaster(p_fb, &tx_packet);

        if (status == serota_BUSY && tx_packet.header.type == serota_MSG_DATA) {
            printf("[OTA MASTER] Dang truyen goi DATA %lu...\n", (unsigned long)tx_packet.header.pack_index);
        }
    }

    if (status == serota_OK) {
        printf("[OTA MASTER] ==> CAP NHAT OTA THANH CONG! Node dich dang khoi dong lai.\n");
        return sercli_OK;
    } else {
        printf("[OTA MASTER] ==> CAP NHAT OTA THAT BAI! (Timeout hoac nhan NACK)\n");
        return sercli_ERROR;
    }
}

sercli_Status cmdHello(const uint8_t argc, const char * const argv[])
{
    (void)argc;
    (void)argv;
    printf("\n========================================\n");
    printf("  Hello World from App 2 (Slot 2)!\n");
    printf("  Firmware OTA da chay thanh cong!\n");
    printf("========================================\n");
    return sercli_OK;
}