#include "lmx2592.h"
#include "hardware/spi.h"
#include "stdio.h"



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

void LMX2592::spi_write24(uint8_t address, uint16_t data) {
    
    uint8_t arr[] = {
        address & 0x7F,
        (uint8_t) (data >> 8),
        (uint8_t) (data & 0xFF)
    };
    //printf("writing: addr = %d, data = 0x%x\n", address, data);
    sleep_us(10);
    gpio_put(GPIO_SPI_LMX_CS, 0);
    sleep_us(10);
    spi_write_blocking(SPI_PORT, arr, 3);
    sleep_us(10);
    gpio_put(GPIO_SPI_LMX_CS, 1);
    sleep_us(10);
}

void LMX2592::init_spi() {

    uint8_t data[3]; // 24 bits

    spi_init(SPI_PORT, 500000);
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

    sleep_ms(100);

    


    load_defaults_into_config();
    config_fields.RESET_1b = 1;
    config_fields.FCAL_EN_1b = 0;
    config_fields.MUXOUT_HDRV_1b = 1;
    config_fields.OSC_2X_1b = 1;
    load_values_into_regfile();
    spi_write24(0, regfile[0]);

    write_all_values();

    ///spi_write24(0, 0b0010001000011000); 
    config_fields.RESET_1b = 0;
    config_fields.FCAL_EN_1b = 1;
    load_values_into_regfile();
    spi_write24(0, regfile[0]);

    //spi_write24(0, 1 << 3); 
}

void LMX2592::dump_values() {
    //gpio_put(GPIO_SPI_LMX_CS, 0);
    //spi_write24(0, 0b0010001000010000); 
    //gpio_put(GPIO_SPI_LMX_CS, 1);
    printf("       | ");
    for (int i = 0; i < 16; i++) {
        printf("%2d  | ", 15 - i);
    }

    uint8_t addr = 0;
    for (addr = 0; addr < 71; addr++) {
        gpio_put(GPIO_SPI_LMX_CS, 0);
        uint8_t cmd = addr | (1 << 7); // READ mode
        uint8_t read_contents[2];
        spi_write_blocking(SPI_PORT, &cmd, 1);
        spi_read_blocking(SPI_PORT, 0, read_contents, 2);
        gpio_put(GPIO_SPI_LMX_CS, 1);
        sleep_ms(1);
        uint16_t contents_merged = (uint16_t)read_contents[1] | ((uint16_t)read_contents[0] << 8); 
        //printf("read values: R%d: 0x%x\n", addr, contents_merged);
        printf("\nR%2d: = |  ", addr);
        for (int i = 0; i < 16; i++) {
            int bit = (contents_merged & (1 << (15 - i))) > 0;
            printf("%d  |  ", bit);
        }
    }
    printf("\n\n\n");
}

void LMX2592::write_all_values() {
    for (int i = 70; i >= 0; i--) {
        if (write_detect[i]) {
            spi_write24(i & 0xff, regfile[i]);
        }
    }
}

void LMX2592::load_values_into_regfile() {
    for (int i = 0; i < 71; i++) {
        regfile[i] = 0;
        write_detect[i] = false;
    }
    // R0
    regfile[0] = 0b0000001000000000;
    regfile[0] |= ((config_fields.POWERDOWN_1b & 0x1) << 0);
    regfile[0] |= ((config_fields.RESET_1b & 0x1) << 1);
    regfile[0] |= ((config_fields.MUXOUT_SEL_1b & 0x1) << 2);
    regfile[0] |= ((config_fields.FCAL_EN_1b & 0x1) << 3);
    regfile[0] |= ((config_fields.ACAL_EN_1b & 0x1) << 4);
    regfile[0] |= ((config_fields.FCAL_LPFD_ADJ_2b & 0x3) << 5);
    regfile[0] |= ((config_fields.FCAL_HPFD_ADJ_2b & 0x3) << 7);
    regfile[0] |= ((config_fields.LD_EN_1b & 0x1) << 13);

    // R1
    regfile[1] = 0b0000100000001000;
    regfile[1] |= ((config_fields.CAL_CLK_DIV_3b & 0x7) << 0);

    // R2
    regfile[2] = 0b0000010100000000;

    // R4
    regfile[4] = 0b0000000001000011;
    regfile[4] |= ((config_fields.ACAL_CMP_DLY_8b & 0xff) << 8);

    // R7
    regfile[7] = 0b0010100010110010;

    // R8
    regfile[8] = 0b0001000010000100;
    regfile[8] |= ((config_fields.VCO_CAPCTRL_OVR_1b & 0x1) << 10);
    regfile[8] |= ((config_fields.VCO_IDAC_OVR_1b & 0x1) << 13);

    // R9
    regfile[9] = 0b0000000100000010;
    regfile[9] |= ((config_fields.REF_EN_1b & 0x1) << 9);
    regfile[9] |= ((config_fields.OSC_2X_1b & 0x1) << 11);

    // R10
    regfile[10] = 0b0001000001011000;
    regfile[10] |= ((config_fields.MULT_5b & 0x1f) << 7);

    // R11
    regfile[11] = 0b0000000000001000;
    regfile[11] |= ((config_fields.PLL_R_8b & 0xff) << 4);

    // R12
    regfile[12] = 0b0111000000000000;
    regfile[12] |= ((config_fields.PLL_R_PRE_12b & 0xfff) << 0);

    // R13
    regfile[13] = 0b0000000000000000;
    regfile[13] |= ((config_fields.PFD_CTL_2b & 0x3) << 0);
    regfile[13] |= ((config_fields.CP_EN_1b & 0x1) << 14);

    // R14
    regfile[14] = 0b0000000000000000;
    regfile[14] |= ((config_fields.CP_ICOARSE_2b & 0x3) << 0);
    regfile[14] |= ((config_fields.CP_IUP_5b & 0x1f) << 2);
    regfile[14] |= ((config_fields.CP_IDN_5b & 0x1f) << 7);

    // R19
    regfile[19] = 0b0000000000000101;
    regfile[19] |= ((config_fields.VCO_IDAC_9b & 0x1ff) << 3);

    // R20
    regfile[20] = 0b0000000000000000;
    regfile[20] |= ((config_fields.ACAL_VCO_IDAC_STRT_9b & 0x1ff) << 0);

    // R22
    regfile[22] = 0b0010001100000000;
    regfile[22] |= ((config_fields.VCO_CAPCTRL_8b & 0xff) << 0);

    // R23
    regfile[23] = 0b1000000001000010;
    regfile[23] |= ((config_fields.VCO_SEL_FORCE_1b & 0x1) << 10);
    regfile[23] |= ((config_fields.VCO_SEL_3b & 0x7) << 11);
    regfile[23] |= ((config_fields.FCAL_VCO_SEL_STRT_1b & 0x1) << 14);

    // R24
    regfile[24] = 0b0000010100001001;

    // R25
    regfile[25] = 0b0000000000000000;

    // R28
    regfile[28] = 0b0010100100100100;

    // R29
    regfile[29] = 0b0000000010000100;

    // R30
    regfile[30] = 0b0000000000110100;
    regfile[30] |= ((config_fields.VCO_2X_EN_1b & 0x1) << 0);
    regfile[30] |= ((config_fields.VTUNE_ADJ_2b & 0x3) << 6);
    regfile[30] |= ((config_fields.MASH_DITHER_1b & 0x1) << 10);

    // R31
    regfile[31] = 0b0000000000000001;
    regfile[31] |= ((config_fields.CHDIV_DIST_PD_1b & 0x1) << 7);
    regfile[31] |= ((config_fields.VCO_DISTA_PD_1b & 0x1) << 9);
    regfile[31] |= ((config_fields.VCO_DISTB_PD_1b & 0x1) << 10);

    // R32
    regfile[32] = 0b0010000100001010;

    // R33
    regfile[33] = 0b0010101000001010;

    // R34
    regfile[34] = 0b1100001111001010;
    regfile[34] |= ((config_fields.CHDIV_EN_1b & 0x1) << 5);

    // R35
    regfile[35] = 0b0000000000011001;
    regfile[35] |= ((config_fields.CHDIV_SEG1_EN_1b & 0x1) << 1);
    regfile[35] |= ((config_fields.CHDIV_SEG1_1b & 0x1) << 2);
    regfile[35] |= ((config_fields.CHDIV_SEG2_EN_1b & 0x1) << 7);
    regfile[35] |= ((config_fields.CHDIV_SEG3_EN_1b & 0x1) << 8);
    regfile[35] |= ((config_fields.CHDIV_SEG2_4b & 0xf) << 9);

    // R36
    regfile[36] = 0b0000000000000000;
    regfile[36] |= ((config_fields.CHDIV_SEG3_3b & 0x7) << 0);
    regfile[36] |= ((config_fields.CHDIV_SEG_SEL_4b & 0xf) << 3);
    regfile[36] |= ((config_fields.CHDIV_DISTA_EN_1b & 0x1) << 10);
    regfile[36] |= ((config_fields.CHDIV_DISTB_EN_1b & 0x1) << 11);

    // R37
    regfile[37] = 0b0100000000000000;
    regfile[37] |= ((config_fields.PLL_N_PRE_1b & 0x1) << 12);

    // R38
    regfile[38] = 0b0000000000000000;
    regfile[38] |= ((config_fields.PLL_N_12b & 0xfff) << 1);

    // R39
    regfile[39] = 0b1000000000000100;
    regfile[39] |= ((config_fields.PFD_DLY_6b & 0x3f) << 8);

    // R40
    regfile[40] = 0b0000000000000000;
    regfile[40] |= ((config_fields.PLL_DEN_31_16__16b & 0xffff) << 0);

    // R41
    regfile[41] = 0b0000000000000000;
    regfile[41] |= ((config_fields.PLL_DEN_15_0__16b & 0xffff) << 0);

    // R42
    regfile[42] = 0b0000000000000000;
    regfile[42] |= ((config_fields.MASH_SEED_31_16__16b & 0xffff) << 0);

    // R43
    regfile[43] = 0b0000000000000000;
    regfile[43] |= ((config_fields.MASH_SEED_15_0__16b & 0xffff) << 0);

    // R44
    regfile[44] = 0b0000000000000000;
    regfile[44] |= ((config_fields.PLL_NUM_31_16__16b & 0xffff) << 0);

    // R45
    regfile[45] = 0b0000000000000000;
    regfile[45] |= ((config_fields.PLL_NUM_15_0__16b & 0xffff) << 0);

    // R46
    regfile[46] = 0b0000000000100000;
    regfile[46] |= ((config_fields.MASH_ORDER_3b & 0x7) << 0);
    regfile[46] |= ((config_fields.OUTA_PD_1b & 0x1) << 6);
    regfile[46] |= ((config_fields.OUTB_PD_1b & 0x1) << 7);
    regfile[46] |= ((config_fields.OUTA_POW_6b & 0x3f) << 8);

    // R47
    regfile[47] = 0b0000000011000000;
    regfile[47] |= ((config_fields.OUTB_POW_6b & 0x3f) << 0);
    regfile[47] |= ((config_fields.OUTA_MUX_2b & 0x3) << 11);

    // R48
    regfile[48] = 0b0000001111111100;
    regfile[48] |= ((config_fields.OUTB_MUX_2b & 0x3) << 0);

    // R59
    regfile[59] = 0b0000000000000000;
    regfile[59] |= ((config_fields.MUXOUT_HDRV_1b & 0x1) << 5);

    // R61
    regfile[61] = 0b0000000000000000;
    regfile[61] |= ((config_fields.LD_TYPE_1b & 0x1) << 0);

    // R62
    regfile[62] = 0b0000000000000000;

    // R64
    regfile[64] = 0b0000000000010000;
    regfile[64] |= ((config_fields.FJUMP_SIZE_4b & 0xf) << 0);
    regfile[64] |= ((config_fields.AJUMP_SIZE_3b & 0x7) << 5);
    regfile[64] |= ((config_fields.FCAL_FAST_1b & 0x1) << 8);
    regfile[64] |= ((config_fields.ACAL_FAST_1b & 0x1) << 9);

    // R68
    regfile[68] = 0b0000000000000000;
    regfile[68] |= ((config_fields.rb_VCO_SEL_3b & 0x7) << 5);
    regfile[68] |= ((config_fields.rb_LD_VTUNE_2b & 0x3) << 9);

    // R69
    regfile[69] = 0b0000000000000000;
    regfile[69] |= ((config_fields.rb_VCO_CAPCTRL_8b & 0xff) << 0);

    // R70
    regfile[70] = 0b0000000000000000;
    regfile[70] |= ((config_fields.rb_VCO_DACISET_9b & 0x1ff) << 0);

    // mark the ones to be written
    write_detect[0] = true;
    write_detect[1] = true;
    write_detect[2] = true;
    write_detect[4] = true;
    write_detect[7] = true;
    write_detect[8] = true;
    write_detect[9] = true;
    write_detect[10] = true;
    write_detect[11] = true;
    write_detect[12] = true;
    write_detect[13] = true;
    write_detect[14] = true;
    write_detect[19] = true;
    write_detect[20] = true;
    write_detect[22] = true;
    write_detect[23] = true;
    write_detect[24] = true;
    write_detect[25] = true;
    write_detect[28] = true;
    write_detect[29] = true;
    write_detect[30] = true;
    write_detect[31] = true;
    write_detect[32] = true;
    write_detect[33] = true;
    write_detect[34] = true;
    write_detect[35] = true;
    write_detect[36] = true;
    write_detect[37] = true;
    write_detect[38] = true;
    write_detect[39] = true;
    write_detect[40] = true;
    write_detect[41] = true;
    write_detect[42] = true;
    write_detect[43] = true;
    write_detect[44] = true;
    write_detect[45] = true;
    write_detect[46] = true;
    write_detect[47] = true;
    write_detect[48] = true;
    write_detect[59] = true;
    write_detect[61] = true;
    write_detect[62] = true;
    write_detect[64] = true;
}

void LMX2592::load_defaults_into_config() {
    // R0
    config_fields.POWERDOWN_1b = 0;
    config_fields.RESET_1b = 0;
    config_fields.MUXOUT_SEL_1b = 1;
    config_fields.FCAL_EN_1b = 0; // this is not default but we want to do this last.
    config_fields.ACAL_EN_1b = 1;
    config_fields.FCAL_LPFD_ADJ_2b = 0;
    config_fields.FCAL_HPFD_ADJ_2b = 0;
    config_fields.LD_EN_1b = 1;
    // R1
    config_fields.CAL_CLK_DIV_3b = 3;
    // R2
    // R4
    config_fields.ACAL_CMP_DLY_8b = 25;
    // R7
    // R8
    config_fields.VCO_CAPCTRL_OVR_1b = 0;
    config_fields.VCO_IDAC_OVR_1b = 0;
    // R9
    config_fields.REF_EN_1b = 1;
    config_fields.OSC_2X_1b = 0;
    // R10
    config_fields.MULT_5b = 1;
    // R11
    config_fields.PLL_R_8b = 1;
    // R12
    config_fields.PLL_R_PRE_12b = 1;
    // R13
    config_fields.PFD_CTL_2b = 0;
    config_fields.CP_EN_1b = 1;
    // R14
    config_fields.CP_ICOARSE_2b = 1;
    config_fields.CP_IUP_5b = 3;
    config_fields.CP_IDN_5b = 3;
    // R19
    config_fields.VCO_IDAC_9b = 300;
    // R20
    config_fields.ACAL_VCO_IDAC_STRT_9b = 300;
    // R22
    config_fields.VCO_CAPCTRL_8b = 0;
    // R23
    config_fields.VCO_SEL_FORCE_1b = 0;
    config_fields.VCO_SEL_3b = 1;
    config_fields.FCAL_VCO_SEL_STRT_1b = 0;
    // R24
    // R25
    // R28
    // R29
    // R30
    config_fields.VCO_2X_EN_1b = 0;
    config_fields.VTUNE_ADJ_2b = 0; // Change this register field according to the VCO frequency, 0: fVCO < 6500 MHz, 3: fVCO ≥ 6500 MHz
    config_fields.MASH_DITHER_1b = 0;
    // R31
    config_fields.CHDIV_DIST_PD_1b = 0;
    config_fields.VCO_DISTA_PD_1b = 0;
    config_fields.VCO_DISTB_PD_1b = 1;
    // R32
    // R33
    // R34
    config_fields.CHDIV_EN_1b = 1;
    // R35
    config_fields.CHDIV_SEG1_EN_1b = 0;
    config_fields.CHDIV_SEG1_1b = 1;
    config_fields.CHDIV_SEG2_EN_1b = 0;
    config_fields.CHDIV_SEG3_EN_1b = 0;
    config_fields.CHDIV_SEG2_4b = 1;
    // R36
    config_fields.CHDIV_SEG3_3b = 1;
    config_fields.CHDIV_SEG_SEL_4b = 1;
    config_fields.CHDIV_DISTA_EN_1b = 1;
    config_fields.CHDIV_DISTB_EN_1b = 0;
    // R37
    config_fields.PLL_N_PRE_1b = 0;
    // R38
    config_fields.PLL_N_12b = 27;
    // R39
    config_fields.PFD_DLY_6b = 2;
    // R40
    config_fields.PLL_DEN_31_16__16b = 1000;
    // R41
    config_fields.PLL_DEN_15_0__16b = 1000;
    // R42
    config_fields.MASH_SEED_31_16__16b = 0;
    // R43
    config_fields.MASH_SEED_15_0__16b = 0;
    // R44
    config_fields.PLL_NUM_31_16__16b = 0;
    // R45
    config_fields.PLL_NUM_15_0__16b = 0;
    // R46
    config_fields.MASH_ORDER_3b = 3;
    config_fields.OUTA_PD_1b = 0;
    config_fields.OUTB_PD_1b = 1;
    config_fields.OUTA_POW_6b = 15;
    // R47
    config_fields.OUTB_POW_6b = 0;
    config_fields.OUTA_MUX_2b = 0;
    // R48
    config_fields.OUTB_MUX_2b = 0;
    // R59
    config_fields.MUXOUT_HDRV_1b = 0;
    // R61
    config_fields.LD_TYPE_1b = 1;
    // R62
    // R64
    config_fields.FJUMP_SIZE_4b = 15;
    config_fields.AJUMP_SIZE_3b = 3;
    config_fields.FCAL_FAST_1b = 0;
    config_fields.ACAL_FAST_1b = 0;
    // R68
    config_fields.rb_VCO_SEL_3b = 0; // these ones are readback values!!
    config_fields.rb_LD_VTUNE_2b = 0; // these ones are readback values!!
    // R69
    config_fields.rb_VCO_CAPCTRL_8b = 0; // these ones are readback values!!
    // R70
    config_fields.rb_VCO_DACISET_9b = 0; // these ones are readback values!!
}