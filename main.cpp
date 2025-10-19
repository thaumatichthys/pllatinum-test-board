#include "stdio.h" // for printf
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/bootrom.h"
#include "string.h"
#include <cstdlib>

#include "lmx2592.h"

/*
    In case of build issues after copying the template, delete the entire build directory and in VSCode:
    Restart VSCode, click on "Terminal" on the top bar, and select GCC 10.2.1 arm-none-eabi

    Overclocking: https://youtu.be/G2BuoFNLoDM (The "catch" is already fixed in CMakeLists.txt, uncomment the three lines at the bottom)
    PIO: https://youtu.be/JSis2NU65w8
    Explaining PIO ASM instructions: https://youtu.be/yYnQYF_Xa8g

    - Max X, 2022-01-07
*/

#define GPIO_RGB_B      25
#define GPIO_RGB_G      16
#define GPIO_RGB_R      17

LMX2592 pll;

void get_inputs() {
    // Fixed-size buffers
    const int MAX_LINE = 128;
    const int MAX_ARGS = 16;

    char line[MAX_LINE];
    char* argv[MAX_ARGS]; // pointers to tokens
    int argc;

    int  param_p = 0;
    int  flag_b = 0;
    char name[64] = {0}; // fixed buffer for string

    printf("\n");
    if (!fgets(line, sizeof(line), stdin)) {
        printf("Error reading input\n");
        //return 1;
    }

    printf("< %s", line);

    // Tokenize line in-place (no allocation)
    argc = 0;
    char* token = strtok(line, " \t\r\n");
    while (token != nullptr && argc < MAX_ARGS) {
        argv[argc++] = token;
        token = strtok(nullptr, " \t\r\n");
    }

    // Parse tokens
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-help") == 0) {
            printf("Usage:\n");    
            printf("  -f <float>    Set frequency in MHz [20.0, 9800.0]\n");
            printf("  -p <int>      Set RF power [0, 47]\n");
            printf("  -rf1 <on/off> Enable RF1\n");
            printf("  -rf2 <on/off> Enable RF2\n");
            printf("  -d <bin/hex>  Dump LMX2592 registers\n");
            printf("  -reboot       Reboot RP2040 into USB boot mode (for reprogramming)\n");
            printf("  -about        About this board\n");
            printf("If you don't see an output, make sure to enable an output channel first!\n");
            continue;
        }

        else if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 < argc) {
                double arg = atof(argv[++i]);
                if (pll.set_frequency(arg * 1'000'000.0)) {
                    printf("> Frequency set to %f MHz\n", arg);
                    sleep_ms(50);
                    
                    uint64_t start_time = to_us_since_boot(get_absolute_time());
                    int timeout = 10000; // 10 ms
                    uint64_t delta_time;
                    bool locked = true;
                    while(!pll.is_locked()) {
                        sleep_us(10);
                        delta_time = to_us_since_boot(get_absolute_time()) - start_time;
                        if (delta_time > timeout) {
                            printf("> PLL could not lock. Maybe there is a problem\n");
                            locked = false;
                            break;
                        }
                    }
                    if (locked)
                        printf("> PLL locked successfully after %d us\n", delta_time);  
                } 
                else {
                    printf("> Error: frequency out of bounds\n");
                }
            }
            else {
                printf("> Usage: -f <frequency in MHz>\n> Example: -f 2400.5\n");
            }
        }
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            if (i + 1 < argc) {
                int arg = atoi(argv[++i]);
                if (pll.set_power_int(arg)) {
                    printf("> Power set to setting %d\n", arg);
                }
                else {
                    printf("> Error: power out of bounds (max 47)\n");
                }
            }
            else {
                printf("> Usage: -p <power setting>\n> Example: -p 15\n");
            }
        }
        else if (strcmp(argv[i], "-rf1") == 0) {
            if (i + 1 < argc) {
                if ((strcmp(argv[i + 1], "on") == 0) || (strcmp(argv[i + 1], "ON") == 0)) {
                    pll.enable_rf1(1);
                    printf("> RF channel 1 ON\n");
                }
                else if ((strcmp(argv[i + 1], "off") == 0) || (strcmp(argv[i + 1], "OFF") == 0)) {
                    pll.enable_rf1(0);
                    printf("> RF channel 1 OFF\n");
                }
                else {
                    printf("> Usage: -rf1 <on/off>\n> Example: -rf1 on\n");
                }
            }
            else {
                printf("> Usage: -rf1 <on/off>\n> Example: -rf1 on\n");
            }
            i++;
        }
        else if (strcmp(argv[i], "-rf2") == 0) {
            if (i + 1 < argc) {
                if ((strcmp(argv[i + 1], "on") == 0) || (strcmp(argv[i + 1], "ON") == 0)) {
                    pll.enable_rf2(1);
                    printf("> RF channel 2 ON\n");
                }
                else if ((strcmp(argv[i + 1], "off") == 0) || (strcmp(argv[i + 1], "OFF") == 0)) {
                    pll.enable_rf2(0);
                    printf("> RF channel 2 OFF\n");
                }
                else {
                    printf("> Usage: -rf2 <on/off>\n> Example: -rf2 on\n");
                }
            }
            else {
                printf("> Usage: -rf2 <on/off>\n> Example: -rf2 on\n");
            }
            i++;
        }
        else if (strcmp(argv[i], "-d") == 0) {
            if (i + 1 < argc) {
                if ((strcmp(argv[i + 1], "hex") == 0) || (strcmp(argv[i + 1], "h") == 0)) {
                    pll.dump_values(true);
                }
                else if ((strcmp(argv[i + 1], "bin") == 0) || (strcmp(argv[i + 1], "b") == 0)) {
                    pll.dump_values(false);
                }
                else {
                    printf("> Usage: -d <hex/bin>\n> Example: -d hex\n");
                }
            }
            else {
                printf("> Usage: -d <hex/bin>\n> Example: -d hex\n");
            }
            printf("> Dumped registers\n");
            i++;
        }
        else if (strcmp(argv[i], "-reboot") == 0) {
            printf("> Rebooting into USB boot\n");
            reset_usb_boot(0, 0);
        }
        else if (strcmp(argv[i], "-about") == 0) {
            printf("> LMX2592 Test Board\n");
            printf("> REF CLK = 48 MHz TCXO, 0.5ppm, Fpfd = REF * 5 / 2 = 120 MHz\n");
            printf("> Frequency range: 20 MHz to 9800 MHz\n");
            printf("> Fundamental (no subharmonics) range: 20 MHz to 7100 MHz. Above that, there will be 1/2 n harmonics due to the doubler.\n");
            printf("> Power draw from USB: ~400 mA\n");
            printf("> Produced October 18, 2025 MX\n");
        }
        else if (strcmp(argv[i], "-thaumatichthys") == 0) {
            printf("hi, how are you?\nthis is the board that I ordered in a rush after the LMX2594 went out of stock on LCSC.\n");
            printf("I really didn't think that it was going to come back any time soon, so I compromised and ordered the LMX2592 instead\n");
            printf("since it was pin-compatible, but just with a much crappier frequency range (even with a doubler!)\n");
            printf("> LMX2592 test board, designed by Max Xiang / thaumatichthys pagidostomus\n> October 19, 2025 at 4am\n> thaumati.com\n");
        }
        else {
            printf("> Unknown command: %s\n> For a list of commands, use -help", argv[i]);
        }
    }
}

int main() {
    
    stdio_init_all(); // for printf

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    gpio_init(GPIO_RGB_G);
    gpio_set_dir(GPIO_RGB_G, GPIO_OUT);
    gpio_init(GPIO_RGB_R);
    gpio_set_dir(GPIO_RGB_R, GPIO_OUT);

    gpio_put(GPIO_RGB_R, 0);
    gpio_put(GPIO_RGB_G, 0);

    sleep_ms(100);
    //printf("he;;p wprld\n");
    pll.init_spi();
    pll.set_frequency(1'100'000'000);
    pll.set_power_int(0);
    pll.enable_rf1(0);
    pll.enable_rf2(0);

    while(1) { // rekt noob timeam
        get_inputs();
    }
}