#include "stdio.h" // for printf
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

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



int main() {
    
    stdio_init_all(); // for printf

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    gpio_init(GPIO_RGB_G);
    gpio_set_dir(GPIO_RGB_G, GPIO_OUT);
    gpio_init(GPIO_RGB_R);
    gpio_set_dir(GPIO_RGB_R, GPIO_OUT);

    gpio_put(GPIO_RGB_R, 1);
    gpio_put(GPIO_RGB_G, 1);
    //gpio_init(GPIO_LMX_MUXOUT);
    //gpio_set_dir(GPIO_LMX_MUXOUT, GPIO_IN);
    
    sleep_ms(100);
    printf("he;;p wprld\n");
    pll.init_spi();

    while(1) { // rekt noob timeam
        gpio_put(25, 1);
        sleep_ms(20);
        gpio_put(25, 0);
        sleep_ms(2000);
        pll.dump_values();
       // gpio_put(GPIO_RGB_R, gpio_get(GPIO_LMX_MUXOUT));
       // gpio_put(GPIO_RGB_G, gpio_get(GPIO_LMX_MUXOUT));
    }
}