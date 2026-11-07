#include "stdlib.h"
#include "string.h"
#include "driver/uart/drvuart.h"
#include "driver/flash/drvflash.h"
#include "service/cli/serCli.h"
#include "service/logging/serLog.h"


extern drvuart_Handle uartHandleDebug; // Declare the debug UART handle as external
extern drvuart_Handle uartHandleOTA; // Declare the OTA UART handle as external

sercli_Status cmdHelp(const uint8_t argc, const char * const argv[]);
sercli_Status cmdTest(const uint8_t argc, const char * const argv[]);

sercli_Status cmdFlashErase(const uint8_t argc, const char * const argv[]);
sercli_Status cmdFlashWrite(const uint8_t argc, const char * const argv[]);
sercli_Status cmdFlashGetInfo(const uint8_t argc, const char * const argv[]);

const sercli_CommandStruct command_list[] = {
    {"help", cmdHelp, "Display this help message"},
    {"test", cmdTest, "Test command for demonstration"},
    {"flash_erase", cmdFlashErase, "Erase a specific flash sector. Usage: flash_erase <sector_number>"},
    {"flash_write", cmdFlashWrite, "Write data to a specific flash sector. Usage: flash_write <sector_number> <data>"},
    {"flash_get_info", cmdFlashGetInfo, "Get information about a specific flash sector. Usage: flash_get_info <sector_number>"},
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