#ifndef CLI_H
#define CLI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

//setting
#define CLI_MAX_COMMAND_LENGTH 32
#define CLI_MAX_PARAMETERS 32
#define CLI_MAX_PARAMETER_LENGTH 32



uint8_t CLI_receive(char *command, uint16_t max_char);
uint8_t CLI_transmirt(char *content, uint16_t max_char);
uint8_t CLI_handle_command();


#ifdef __cplusplus
}
#endif

#endif /* CLI_H */