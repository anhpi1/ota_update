#include "cli.h"


/*hãy thiết lập hàm nhận*/
unit8_t CLI_receive()
{

    CLI_handle_command(,1024);
    return 1; // if command success
}

/*hãy thiết lập hàm truyền */
uint8_t CLI_transmirt(char *content, uint16_t max_char)
{
    // TODO: Implement the transmission function
    return 1;
}

uint8_t CLI_handle_command(char *command, uint16_t max_char)
{
    char key[CLI_MAX_COMMAND_LENGTH] = {0};
    char parameters[CLI_MAX_PARAMETERS][CLI_MAX_PARAMETER_LENGTH] = {0};
    uint16_t i = 0;
    uint8_t  j = 0;                  // chỉ số ghi trong key / từng parameter
    uint8_t  n_parameters = 0;

    if (command == NULL || max_char == 0) return 0;

    /* ----- Vòng 1: tách key ----- */
    for (; i < max_char; i++)
    {
        if (command[i] == '\0' || command[i] == '\n' || command[i] == ' ') break;
        if (j < CLI_MAX_COMMAND_LENGTH - 1) key[j++] = command[i];
    }
    key[j] = '\0';
    if(command[i] == '\0') goto end; // nếu kết thúc bằng '\0' thì bỏ qua
    i++; // nhảy qua space

    /* ----- Vòng 2: tách các parameter ----- */
    /* i đang ở space / '\n' / '\0' → nhảy qua space trước */
    for (; i < max_char; i++)
    {
        if (command[i] == '\0' || command[i] == '\n') break; // kết thúc
        if (command[i] == ' ')
        {
            parameters[n_parameters][j] = '\0';
            n_parameters++;
            j = 0;
        }


    }

    /* Nếu kết thúc bằng '\n' hoặc '\0' mà param cuối chưa đóng */
    if (j > 0 && n_parameters < CLI_MAX_PARAMETERS)
    {
        parameters[n_parameters][j] = '\0';
        n_parameters++;
    }

    end:
    /* TODO: dùng key + parameters ở đây */

    return n_parameters;
}