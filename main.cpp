#include "stdio.h" // for printf
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

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

#define GPIO_SPI_MOSI   3
#define GPIO_SPI_SCK    2
#define GPIO_SPI_LMX_CS 1
#define SPI_PORT        spi0

#define GPIO_LMX_SYSREFFREQ 28
#define GPIO_LMX_RAMPCLK    29
#define GPIO_LMX_MUXOUT     4
#define GPIO_LMX_RAMPDIR    6
#define GPIO_LMX_SYNC       7
#define GPIO_LMX_EN         0


static inline uint16_t bitreverse16(uint16_t x) {
    x = (x >> 1)  & 0x5555 | (x & 0x5555) << 1;
    x = (x >> 2)  & 0x3333 | (x & 0x3333) << 2;
    x = (x >> 4)  & 0x0F0F | (x & 0x0F0F) << 4;
    x = (x >> 8)  | (x << 8);
    return x;
}

void spi_write24(uint8_t address, uint16_t data) {
    uint8_t arr[] = {
        address & 0x7F,
        (uint8_t) (data >> 8),
        (uint8_t) (data & 0xFF)
    };
    spi_write_blocking(SPI_PORT, arr, 3);
}

void init_spi() {

    uint8_t data[3]; // 24 bits

    spi_init(SPI_PORT, 5000000);
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(GPIO_SPI_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(GPIO_SPI_SCK, GPIO_FUNC_SPI);
    gpio_set_function(GPIO_LMX_MUXOUT, GPIO_FUNC_SPI);

    gpio_init(GPIO_SPI_LMX_CS);
    gpio_set_dir(GPIO_SPI_LMX_CS, GPIO_OUT);
    gpio_put(GPIO_SPI_LMX_CS, 1);


    gpio_init(GPIO_LMX_EN);
    gpio_set_dir(GPIO_LMX_EN, GPIO_OUT);
    gpio_put(GPIO_LMX_EN, 1);

    sleep_ms(10);

    // soft reset 
    //0b00 x 0001 xx xx x x x x x
    //0b00 1 0001 00 00 1 0 1 1 0
    //gpio_put(GPIO_SPI_LMX_CS, 0);
    //spi_write24(0, 0b0010001000010110); // MUXOUT = lock detect
    //spi_write24(0, 0b0010001000010010); 
    //spi_write24(0, 1 << 1); 
    //gpio_put(GPIO_SPI_LMX_CS, 1);
    //sleep_ms(1);
/*

    // // set PFD_DLY to setting 1:
    // gpio_put(GPIO_SPI_LMX_CS, 0);
    // spi_write24(39, 1 << 8); 
    // gpio_put(GPIO_SPI_LMX_CS, 1);
    // sleep_ms(1);

    // freq cal
    gpio_put(GPIO_SPI_LMX_CS, 0);
    //spi_write24(0, 0b0010001000011100); 
    //  spi_write24(0, 0b0010000000011100); 
    //spi_write24(0, 1 << 3); 
    spi_write24(0, 0b0010001000011100);
    gpio_put(GPIO_SPI_LMX_CS, 1);
    sleep_ms(1);

    // set N divider value
    uint16_t n_divider = 1000;
    gpio_put(GPIO_SPI_LMX_CS, 0);
    spi_write24(38, (n_divider << 1) & 0b0001111111111110); 
    gpio_put(GPIO_SPI_LMX_CS, 1);
    sleep_ms(1);

    
    // set PLL numerator
    uint32_t pll_numerator = 1000;
    gpio_put(GPIO_SPI_LMX_CS, 0);
    spi_write24(44, (uint16_t)(pll_numerator >> 16)); 
    gpio_put(GPIO_SPI_LMX_CS, 1);
    sleep_ms(1);
    gpio_put(GPIO_SPI_LMX_CS, 0);
    spi_write24(45, (uint16_t)(pll_numerator & 0xFFFF)); 
    gpio_put(GPIO_SPI_LMX_CS, 1);

    sleep_ms(1);

    // set PLL denominator
    uint32_t pll_denominator = 1000;
    gpio_put(GPIO_SPI_LMX_CS, 0);
    spi_write24(40, (uint16_t)(pll_denominator >> 16)); 
    gpio_put(GPIO_SPI_LMX_CS, 1);
    sleep_ms(1);
    gpio_put(GPIO_SPI_LMX_CS, 0);
    spi_write24(41, (uint16_t)(pll_denominator & 0xFFFF)); 
    gpio_put(GPIO_SPI_LMX_CS, 1);
    
    
    // freq cal
    gpio_put(GPIO_SPI_LMX_CS, 0);
    spi_write24(0, 1 << 3); 
    gpio_put(GPIO_SPI_LMX_CS, 1);

    sleep_ms(1);*/
}

void dump_values() {
    gpio_put(GPIO_SPI_LMX_CS, 0);
    spi_write24(0, 0b0010001000010000); 
    gpio_put(GPIO_SPI_LMX_CS, 1);
    printf("       | ");
    for (int i = 0; i < 16; i++) {
        printf("%2d  | ", i);
    }

    uint8_t addr = 0;
    for (addr = 0; addr < 60; addr++) {
        gpio_put(GPIO_SPI_LMX_CS, 0);
        uint8_t cmd = addr | (1 << 7); // READ mode
        uint8_t read_contents[2];
        spi_write_blocking(SPI_PORT, &cmd, 1);
        spi_read_blocking(SPI_PORT, 0, read_contents, 2);
        gpio_put(GPIO_SPI_LMX_CS, 1);
        sleep_ms(1);
        uint16_t contents_merged = (uint16_t)read_contents[0] | ((uint16_t)read_contents[1] << 8); 
        //printf("read values: R%d: 0x%x\n", addr, contents_merged);
        printf("\nR%2d: = |  ", addr);
        for (int i = 0; i < 16; i++) {
            int bit = (contents_merged & (1 << (15 - i))) > 0;
            printf("%d  |  ", bit);
        }
    }
    printf("\n\n\n");

    
}

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
    init_spi();

    while(1) { // rekt noob timeam
        gpio_put(25, 1);
        sleep_ms(20);
        gpio_put(25, 0);
        sleep_ms(2000);
        dump_values();
       // gpio_put(GPIO_RGB_R, gpio_get(GPIO_LMX_MUXOUT));
       // gpio_put(GPIO_RGB_G, gpio_get(GPIO_LMX_MUXOUT));
    }
}