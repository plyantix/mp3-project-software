#include <stdio.h>
#include "pico/stdlib.h"

#include "hw_config.h"
#include "ff.h"

int main()
{
    stdio_init_all();

    FATFS fs;
    FIL file;
    FRESULT res;
    char buff[1000];

    sleep_ms(5*1000);
    printf("Attempting to mount file system.\n");


    res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        printf("Could not mount file system! Erro: %d\n", res);
        return 0;
    }
    printf("Sucessfully mounted file system!\n");


    printf("Enter text:\n");

    int index = 0;
    while (true) {
        // Non-blocking read (returns PICO_ERROR_TIMEOUT if no character is sent)
        int c = stdio_getchar();
        
        if (c != PICO_ERROR_TIMEOUT) {
            // Check for line endings
            if (c == '\n' || c == '\r') {
                buff[index] = '\0'; // Terminate string
                if (index > 0) {
                    index = 0; // Reset buffer index
                }
                break;
            } else if (index < sizeof(buff) - 1) {
                buff[index++] = (char)c; // Store character
            }
        }
    }
    sleep_ms(10);
    puts_raw(buff);
    
    f_open(&file, "test.txt", FA_OPEN_APPEND | FA_WRITE);

    f_puts(buff, &file);

    f_close(&file);

    f_open(&file, "test.txt", FA_READ);

    f_gets(buff, sizeof(buff), &file);

    printf(buff);
}
