#ifndef __ADBMS1818_H__
#define __ADBMS1818_H__

#include "stdint.h"
#include "spi.h"
#include "stdbool.h"

#define I2C_DATA(ICOM, DATA, FCOM) ((uint16_t)(ICOM) << 8 | (uint16_t)(DATA) << 4 | (uint16_t)(FCOM))


typedef enum {
    DCTO_DISABLED,
    DCTO_0_5_MIN,
    DCTO_1_MIN,
    DCTO_2_MIN,
    DCTO_3_MIN,
    DCTO_4_MIN,
    DCTO_5_MIN,
    DCTO_10_MIN,
    DCTO_15_MIN,
    DCTO_20_MIN,
    DCTO_30_MIN,
    DCTO_40_MIN,
    DCTO_60_MIN,
    DCTO_75_MIN,
    DCTO_90_MIN,
    DCTO_120_MIN,
}adbms1818_dcto_t;



typedef enum {
    // ICOM_W
    I2C_START       = 0x60,
    I2C_STOP        = 0x10,
    I2C_BLANK       = 0x00,
    I2C_NO_TRANSMIT = 0x70,
    // FCOM_W
    I2C_M_ACK       = 0x00,
    I2C_M_NAK       = 0x08,
    I2C_M_NAK_STOP  = 0x09
}adbms1818_i2c_w_t;


typedef enum {
    BMS_ERR,
    BMS_BUSY,
    BMS_IO_ERR,
    BMS_OK
}bms_status_type_t;


typedef enum {
    ADC_GPIO_ALL,
    ADC_GPIO_1_6,
    ADC_GPIO_2_7,
    ADC_GPIO_3_8,
    ADC_GPIO_4_9,
    ADC_GPIO_5,
    ADC_GPIO_REF2,
}adbms1818_adc_gpio_t;


typedef enum {
    OPT0_422_OPT1_1K    = 0 << 7,
    OPT0_27K_OPT1_14K   = 1 << 7,
    OPT0_7K_OPT1_3K     = 2 << 7,
    OPT0_26_OPT1_2K     = 3 << 7
}adbms1818_adc_mode_t;


typedef struct {
    SPI_HandleTypeDef *hspi;
    uint8_t ic_count;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    GPIO_TypeDef *sdo_port;
    uint16_t sdo_pin;
}adbms1818_handle_t;


typedef struct {
    uint32_t VUV;   // micro voltages
    uint32_t VOV;   // micro voltages
    uint32_t DCC;   // bit 18:1 -> cell 18:1, bit 0 -> IO9 pull down
    adbms1818_dcto_t DCTO;
    uint16_t GPIO;
    uint8_t PS;
    bool REFON;
    bool DTEN;
    bool ADCOPT;
    bool MUTE;
    bool FDRF;
    bool DTMEN;
    bool PEC_FAILED;
}adbms1818_cfg_t;


typedef struct {
    uint32_t SC;        // micro voltages
    uint32_t VA;        // micro voltages
    uint32_t VD;        // micro voltages
    float ITMP;         // celsius
    uint8_t OUV_FLAG[18];   // 0: no flag, 1: uv flag, 2: ov flag, 3: both flag(should not happened)
    uint8_t REV;
    uint8_t MUXFAIL;
    uint8_t THSD;
    uint8_t PEC_FAILED;
}adbms1818_stats_t;

void wake_up(const adbms1818_handle_t *bms);

void adbms1818_get_default_cfg(const adbms1818_handle_t *bms, adbms1818_cfg_t cfg[]);


void adbms1818_handle_init(adbms1818_handle_t *bms, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin,
    GPIO_TypeDef *sdo_port, uint16_t sdo_pin, uint8_t ic_count);

bms_status_type_t adbms1818_read_cfg(const adbms1818_handle_t *bms, adbms1818_cfg_t *cfg);

bms_status_type_t adbms1818_read_stats(const adbms1818_handle_t *bms, adbms1818_stats_t *stats);

bms_status_type_t adbms1818_start_i2c(const adbms1818_handle_t *bms, uint16_t data[3], uint16_t i2c_real_size);

bms_status_type_t adbms1818_write_cfg(const adbms1818_handle_t *bms, const adbms1818_cfg_t *cfg);

bms_status_type_t adbms1818_stats_adc_conversion(const adbms1818_handle_t *bms, adbms1818_adc_mode_t adc_mode, uint32_t wait_time);

bms_status_type_t adbms1818_gpio_adc_conversion(const adbms1818_handle_t *bms, adbms1818_adc_gpio_t adc_gpio, 
    adbms1818_adc_mode_t adc_mode, uint32_t wait_time);

bms_status_type_t adbms1818_cell_adc_conversion(const adbms1818_handle_t *bms, adbms1818_adc_mode_t adc_mode, uint32_t wait_time);

bms_status_type_t adbms1818_open_wire_adc_conversion(const adbms1818_handle_t *bms, bool pup);

bms_status_type_t adbms1818_get_gpio_voltage(const adbms1818_handle_t *bms, uint32_t voltage[][9], uint32_t ref_voltage[]);

bms_status_type_t adbms1818_get_cell_voltage(const adbms1818_handle_t *bms, uint32_t voltage[][18]);

bms_status_type_t adbms1818_adc_handle(const adbms1818_handle_t *bms, bool blocking);

bms_status_type_t adbms1818_open_wire_test(const adbms1818_handle_t *bms, bool success[][18]);


void test();



#endif