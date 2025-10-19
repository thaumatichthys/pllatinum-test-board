#pragma once 
#include "pico/stdlib.h"

struct lmx2592_fields {
    // R0
    uint16_t POWERDOWN_1b;
    uint16_t RESET_1b;
    uint16_t MUXOUT_SEL_1b;
    uint16_t FCAL_EN_1b;
    uint16_t ACAL_EN_1b;
    uint16_t FCAL_LPFD_ADJ_2b;
    uint16_t FCAL_HPFD_ADJ_2b;
    uint16_t LD_EN_1b;

    // R1
    uint16_t CAL_CLK_DIV_3b;

    // R2

    // R4
    uint16_t ACAL_CMP_DLY_8b;

    // R7

    // R8
    uint16_t VCO_CAPCTRL_OVR_1b;
    uint16_t VCO_IDAC_OVR_1b;

    // R9
    uint16_t REF_EN_1b;
    uint16_t OSC_2X_1b;

    // R10
    uint16_t MULT_5b;

    // R11
    uint16_t PLL_R_8b;

    // R12
    uint16_t PLL_R_PRE_12b;

    // R13
    uint16_t PFD_CTL_2b;
    uint16_t CP_EN_1b;

    // R14
    uint16_t CP_ICOARSE_2b;
    uint16_t CP_IUP_5b;
    uint16_t CP_IDN_5b;

    // R19
    uint16_t VCO_IDAC_9b;

    // R20
    uint16_t ACAL_VCO_IDAC_STRT_9b;

    // R22
    uint16_t VCO_CAPCTRL_8b;

    // R23
    uint16_t VCO_SEL_FORCE_1b;
    uint16_t VCO_SEL_3b;
    uint16_t FCAL_VCO_SEL_STRT_1b;

    // R24

    // R25

    // R28

    // R29

    // R30
    uint16_t VCO_2X_EN_1b;
    uint16_t VTUNE_ADJ_2b;
    uint16_t MASH_DITHER_1b;

    // R31
    uint16_t CHDIV_DIST_PD_1b;
    uint16_t VCO_DISTA_PD_1b;
    uint16_t VCO_DISTB_PD_1b;

    // R32

    // R33

    // R34
    uint16_t CHDIV_EN_1b;

    // R35
    uint16_t CHDIV_SEG1_EN_1b;
    uint16_t CHDIV_SEG1_1b;
    uint16_t CHDIV_SEG2_EN_1b;
    uint16_t CHDIV_SEG3_EN_1b;
    uint16_t CHDIV_SEG2_4b;

    // R36
    uint16_t CHDIV_SEG3_3b;
    uint16_t CHDIV_SEG_SEL_4b;
    uint16_t CHDIV_DISTA_EN_1b;
    uint16_t CHDIV_DISTB_EN_1b;

    // R37
    uint16_t PLL_N_PRE_1b;

    // R38
    uint16_t PLL_N_12b;

    // R39
    uint16_t PFD_DLY_6b;

    // R40
    uint16_t PLL_DEN_31_16__16b;

    // R41
    uint16_t PLL_DEN_15_0__16b;

    // R42
    uint16_t MASH_SEED_31_16__16b;

    // R43
    uint16_t MASH_SEED_15_0__16b;

    // R44
    uint16_t PLL_NUM_31_16__16b;

    // R45
    uint16_t PLL_NUM_15_0__16b;

    // R46
    uint16_t MASH_ORDER_3b;
    uint16_t OUTA_PD_1b;
    uint16_t OUTB_PD_1b;
    uint16_t OUTA_POW_6b;

    // R47
    uint16_t OUTB_POW_6b;
    uint16_t OUTA_MUX_2b;

    // R48
    uint16_t OUTB_MUX_2b;

    // R59
    uint16_t MUXOUT_HDRV_1b;

    // R61
    uint16_t LD_TYPE_1b;

    // R62

    // R64
    uint16_t FJUMP_SIZE_4b;
    uint16_t AJUMP_SIZE_3b;
    uint16_t FCAL_FAST_1b;
    uint16_t ACAL_FAST_1b;

    // R68
    uint16_t rb_VCO_SEL_3b;
    uint16_t rb_LD_VTUNE_2b;

    // R69
    uint16_t rb_VCO_CAPCTRL_8b;

    // R70
    uint16_t rb_VCO_DACISET_9b;
};

class LMX2592 {
    static constexpr double VCO_MIN_HZ = 3'550'000'000.0;
    static constexpr double VCO_MAX_HZ = 7'100'000'000.0;
    static constexpr double OUT_MAX_HZ = 9'800'000'000.0;
    static constexpr double OUT_MIN_HZ =    20'000'000.0;
    static constexpr double REF_HZ = 48'000'000.0;


    uint16_t regfile[71];
    bool write_detect[71];
public:
    lmx2592_fields config_fields;
    void load_values_into_regfile();
    void load_defaults_into_config();
    void spi_write24(uint8_t address, uint16_t data);
    void init_spi();
    void dump_values();
    void write_all_values();
    void soft_reset();
    void do_fcal();
    void load_divider_into_config(double divider);
    bool set_frequency(double freq_hz);
};