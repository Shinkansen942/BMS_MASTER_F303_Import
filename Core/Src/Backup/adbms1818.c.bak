#include "adbms1818.h"
#include <string.h>
#include <stdlib.h>
#include "config.h"

/***** [Define] *****/

#define CHIP_SELECT(cfg) (HAL_GPIO_WritePin((cfg)->cs_port, (cfg)->cs_pin, GPIO_PIN_RESET))
#define CHIP_UNSELECT(cfg) (HAL_GPIO_WritePin((cfg)->cs_port, (cfg)->cs_pin, GPIO_PIN_SET))
#define HAL_ERR_CHECK(stat) do{ if((stat)!=HAL_OK) {return BMS_IO_ERR;} }while(0);
#define BMS_ERR_CHECK(stat) do{ if((stat)!=BMS_OK) {return (stat);} }while(0);

/***** [Constant] *****/

typedef enum {
    WRCFGA      = 0x0001,
    WRCFGB      = 0x0024,
    RDCFGA      = 0x0002,
    RDCFGB      = 0x0026,
    RDCVA       = 0x0004,
    RDCVB       = 0x0006,
    RDCVC       = 0x0008,
    RDCVD       = 0x000A,
    RDCVE       = 0x0009,
    RDCVF       = 0x000B,
    RDAUXA      = 0x000C,
    RDAUXB      = 0x000E,
    RDAUXC      = 0x000D,
    RDAUXD      = 0x000F,
    RDSTATA     = 0x0010,
    RDSTATB     = 0x0012,
    ADSTAT      = 0x0468,
    ADSTATD     = 0x0468,
    STATST      = 0x040F,
    WRCOMM      = 0x0721,
    RDCOMM      = 0x0722,
    STCOMM      = 0x0723,
    ADAX        = 0x0460,
    ADAXD       = 0x0400,
    ADCV        = 0x0260,
    ADOW        = 0x0228,
    PLADC       = 0x0714
}adbms1818_cmd_t;



/***** [Local] *****/

const uint16_t crc15Table[256] = {  // precomputed CRC15 Table
    0x0,0xc599, 0xceab, 0xb32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac,  
    0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1,
    0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e,
    0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b,
    0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd,
    0x2544, 0x2be, 0xc727, 0xcc15, 0x98c, 0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c,
    0x3d6e, 0xf8f7,0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x7c2, 0xc25b, 0xc969, 0xcf0, 0xdf0d,
    0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf,
    0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640,
    0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba,
    0x4a88, 0x8f11, 0x57c, 0xc0e5, 0xcbd7, 0xe4e, 0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b,
    0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921,
    0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070,
    0x85e9, 0xf84, 0xca1d, 0xc12f, 0x4b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528,
    0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59,
    0x2ac0, 0xd3a, 0xc8a3, 0xc391, 0x608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01,
    0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9,
    0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a,
    0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25,
    0x2fbc, 0x846, 0xcddf, 0xc6ed, 0x374, 0xd089, 0x1510, 0x1e22, 0xdbbb, 0xaf8, 0xcf61, 0xc453,
    0x1ca, 0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b,
    0x2d02, 0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3,
    0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095
};


/* Calculates  and returns the CRC15 */
static uint16_t pec15_calc (
    uint8_t len, //Number of bytes that will be used to calculate a PEC
    uint8_t *data //Array of data that will be used to calculate  a PEC
) {
	uint16_t remainder,addr;
	remainder = 16;//initialize the PEC
	for (uint8_t i = 0; i<len; i++) // loops for each byte in data array
	{
		addr = ((remainder>>7)^data[i])&0xff;//calculate PEC table address
		remainder = (remainder<<8)^crc15Table[addr];
	}
	return(remainder*2);//The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2
}



static bool pec_check(uint8_t *data, uint16_t size) {
    uint16_t pec_data = pec15_calc(size-2, data);
    uint16_t pec_return = data[size-1] | ((uint16_t)data[size-2] << 8);
    return pec_data == pec_return;
}



/**
 * @param data pointer of data
 * @param size data size + 2 (for pec)
*/
void apply_pec(uint8_t *data, uint16_t size) {
    uint16_t pec = pec15_calc(size-2, data);
    data[size-2] = pec >> 8;
    data[size-1] = pec;
}



/**
 * Read data. Data start from the head of isoSPI to the tail of isoSPI
 * 
*/
static bms_status_type_t cmd_read(const adbms1818_handle_t *bms, adbms1818_cmd_t cmd, uint8_t *data, uint16_t size, bool cs_lock) {
    uint16_t tx_size = 4;
    uint8_t tx_buf[tx_size];
    CHIP_SELECT(bms);
    tx_buf[0] = (cmd) >> 8;
    tx_buf[1] = (cmd);
    apply_pec(tx_buf, 4);
    HAL_StatusTypeDef hal_stat = HAL_SPI_Transmit(bms->hspi, tx_buf, tx_size, HAL_MAX_DELAY);
    HAL_ERR_CHECK(hal_stat);
    if(data != NULL && size != 0) {
        hal_stat = HAL_SPI_Receive(bms->hspi, data, size, HAL_MAX_DELAY);
        HAL_ERR_CHECK(hal_stat);
    }
    if(!cs_lock) {
        CHIP_UNSELECT(bms);
    }
    return BMS_OK;
}



/**
 * Write data. Data start from the tail of isoSPI to the head of isoSPI
 * 
*/
static bms_status_type_t cmd_write(const adbms1818_handle_t *bms, adbms1818_cmd_t cmd, uint8_t* data, uint16_t size) {
    uint16_t tx_size = 4 + size;
    uint8_t tx_buf[tx_size];
    CHIP_SELECT(bms);
    tx_buf[0] = (cmd) >> 8;
    tx_buf[1] = (cmd);
    apply_pec(tx_buf, 4);
    if(data != NULL && size != 0) {
        memcpy(&tx_buf[4], data, size);
    }
    HAL_StatusTypeDef hal_stat = HAL_SPI_Transmit(bms->hspi, tx_buf, tx_size, HAL_MAX_DELAY);
    HAL_ERR_CHECK(hal_stat);
    CHIP_UNSELECT(bms);
    return BMS_OK;
}

/***** [Global] *****/

void wake_up(const adbms1818_handle_t *bms) {
    uint16_t size = bms->ic_count * 8;
    uint8_t garbage[size];
    cmd_read(bms, RDCFGA, garbage, size, false);
    HAL_Delay(1);
}


/**
 * @param cfg config array, length should be bms->ic_count
 */
void adbms1818_get_default_cfg(const adbms1818_handle_t *bms, adbms1818_cfg_t cfg[]) {
    for(int ic = 0; ic < bms->ic_count; ic++) {
        cfg[ic] = (adbms1818_cfg_t){
            .VUV        = 3600000,  // 3.6V
            .VOV        = 4100000,  // 4.1V
            .DCC        = 0,        // discharge off
            .DCTO       = 0,        // discharge timeout disable
            .GPIO       = 0x1FF,    // no pull down        
            .PS         = 0,        // redundancy sequentially to all
            .REFON      = false,    // ref off   
            .DTEN       = true,     // ro don't care
            .ADCOPT     = false,    // default 0
            .MUTE       = false,    // ro don't care
            .FDRF       = false,    // normal mode
            .DTMEN      = true,     // enable discharge monitor
            .PEC_FAILED = 0,
        };
    }
}



void adbms1818_handle_init(adbms1818_handle_t *bms, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin,
    GPIO_TypeDef *sdo_port, uint16_t sdo_pin, uint8_t ic_count)
{
    bms->cs_port = cs_port;
    bms->cs_pin = cs_pin;
    bms->sdo_port = sdo_port;
    bms->sdo_pin = sdo_pin;
    bms->hspi = hspi;
    bms->ic_count = ic_count;
}



/**
 * @param cfg poiter of config array -> cfg[ic_count]
 */
bms_status_type_t adbms1818_read_cfg(const adbms1818_handle_t *bms, adbms1818_cfg_t cfg[]) {
    uint16_t size = bms->ic_count * 8;
    bms_status_type_t bms_stat;
    uint8_t data_a[size];
    uint8_t data_b[size];
    uint16_t offset = 0;

    wake_up(bms);
    bms_stat = cmd_read(bms, RDCFGA, data_a, size, false);
    BMS_ERR_CHECK(bms_stat);
    bms_stat = cmd_read(bms, RDCFGB, data_b, size, false);
    BMS_ERR_CHECK(bms_stat);

    for(int ic = 0; ic < bms->ic_count; ic++) { // data begin <=> ic 0
        bool pec_a_ok = pec_check(&data_a[offset], 8);
        bool pec_b_ok = pec_check(&data_b[offset], 8);
        // PEC Check
        if(pec_a_ok && pec_b_ok) {
            cfg[ic].PEC_FAILED = 0;
        }else {
            cfg[ic].PEC_FAILED = 1;
            continue;
        }
        // Parse
        // VUV
        cfg[ic].VUV  = data_a[offset + 1] + 1;                     // [7:0]
        cfg[ic].VUV += (uint16_t)(data_a[offset + 2] & 0x0F) << 8; // [11:8]
        cfg[ic].VUV *= 1600;
        // VOV
        cfg[ic].VOV  = (data_a[offset + 2] >> 4) + 1;       // [3:0]
        cfg[ic].VOV += data_a[offset + 3] << 4;             // [11:4]
        cfg[ic].VOV *= 1600;
        // DCC
        cfg[ic].DCC  = (data_b[offset+1] & 0x04) >> 2;                 // [0]
        cfg[ic].DCC += (uint32_t)data_a[offset + 4] << 1;              // [8:1]
        cfg[ic].DCC += (uint32_t)(data_a[offset + 5] & 0x0F) << 9;     // [12:9]
        cfg[ic].DCC += (uint32_t)(data_b[offset] & 0xF0) << 9;         // [16:13]
        cfg[ic].DCC += (uint32_t)(data_b[offset + 1] & 0x03) << 17;    // [18:17]
        // DCTO
        cfg[ic].DCTO = (data_a[offset + 5] & 0xF0) >> 4;
        // GPIO
        cfg[ic].GPIO  = data_a[offset] >> 3;                       // [5:0]
        cfg[ic].GPIO += (uint32_t)(data_a[offset] & 0x0F) << 6;    // [9:6]
        // PS
        cfg[ic].PS = (data_b[offset + 1] & 0x30) >> 4;
        // REFON
        cfg[ic].REFON = data_a[offset] & 0x04;
        // DTEN
        cfg[ic].DTEN = data_a[offset] & 0x02;
        // ADCOPT
        cfg[ic].ADCOPT = data_a[offset] & 0x01;
        // MUTE
        cfg[ic].MUTE = data_b[offset + 1] & 0x80;
        // FDRF
        cfg[ic].FDRF = data_b[offset + 1] & 0x40;
        // DTMEN
        cfg[ic].DTMEN = data_b[offset + 1] & 0x08;

        offset += 8;
    }
    return BMS_OK;
}



/**
 * @param cfg poiter of config array -> cfg[ic_count]
 */
bms_status_type_t adbms1818_write_cfg(const adbms1818_handle_t *bms, const adbms1818_cfg_t cfg[]) {
    uint16_t size = bms->ic_count * 8;
    bms_status_type_t bms_stat;
    uint8_t data_a[size];
    uint8_t data_b[size];
    uint16_t offset = 0;

    wake_up(bms);
    for(int ic = bms->ic_count-1; ic >= 0; ic--) {  // data end <=> ic 0
        uint16_t VUV = cfg[ic].VUV / 1600 - 1;
        uint16_t VOV = cfg[ic].VOV / 1600 - 1;

        data_a[offset + 0] = cfg[ic].GPIO << 3 | cfg[ic].REFON << 2 | cfg[ic].DTEN << 1 | cfg[ic].ADCOPT;
        data_a[offset + 1] = VUV;
        data_a[offset + 2] = VOV << 4 | VUV >> 8;
        data_a[offset + 3] = VOV >> 4;
        data_a[offset + 4] = cfg[ic].DCC >> 1;
        data_a[offset + 5] = cfg[ic].DCTO << 4 | (cfg[ic].DCC & 0x1E00) >> 9;

        data_b[offset + 0] = (cfg[ic].DCC & 0x1E000) >> 9 | cfg[ic].GPIO >> 5;
        data_b[offset + 1] = cfg[ic].MUTE << 7 | cfg[ic].FDRF << 6 | (cfg[ic].PS & 0x03) << 4 
            | cfg[ic].DTMEN << 3 | (cfg[ic].DCC & 1) << 2 | cfg[ic].DCC >> 17;
        data_b[offset + 2] = 0;
        data_b[offset + 3] = 0;
        data_b[offset + 4] = 0;
        data_b[offset + 5] = 0;

        apply_pec(&data_a[offset], 8);
        apply_pec(&data_b[offset], 8);
        offset += 8;
    }

    bms_stat = cmd_write(bms, WRCFGA, data_a, size);
    BMS_ERR_CHECK(bms_stat);
    bms_stat = cmd_write(bms, WRCFGB, data_b, size);
    BMS_ERR_CHECK(bms_stat);
    return BMS_OK;
}


/**
 * @param stats poiter of stats array -> stats[ic_count]
 */
bms_status_type_t adbms1818_read_stats(const adbms1818_handle_t *bms, adbms1818_stats_t stats[]) {
    uint16_t size = bms->ic_count * 8;
    bms_status_type_t rd_stat;
    uint8_t data_a[size];
    uint8_t data_b[size];
    uint8_t data_aux_d[size];

    wake_up(bms);
    rd_stat = cmd_read(bms, RDSTATA, data_a, size, false);
    BMS_ERR_CHECK(rd_stat);
    rd_stat = cmd_read(bms, RDSTATB, data_b, size, false);
    BMS_ERR_CHECK(rd_stat);
    rd_stat = cmd_read(bms, RDAUXD, data_aux_d, size, false);
    BMS_ERR_CHECK(rd_stat);

    for(int ic = 0; ic < bms->ic_count; ic++) { // data begin <=> ic 0
        uint16_t offset = 8*ic;
        bool pec_a_ok = pec_check(&data_a[offset], 8);
        bool pec_b_ok = pec_check(&data_b[offset], 8);
        bool pec_aux_d_ok = pec_check(&data_aux_d[offset], 8);
        // PEC Check
        if(pec_a_ok && pec_b_ok && pec_aux_d_ok) {
            stats[ic].PEC_FAILED = 0;
        }else {
            memset(&stats[ic], 0, sizeof(stats[ic]));
            stats[ic].PEC_FAILED = 1;
            continue;
        }
        // Parse
        // SC
        stats[ic].SC  = data_a[offset];                     // [7:0]
        stats[ic].SC += (uint32_t)data_a[offset + 1] << 8;  // [15:8]
        stats[ic].SC *= 3000;
        // VA
        stats[ic].VA  = data_a[offset + 4];                 // [7:0]
        stats[ic].VA += (uint32_t)data_a[offset + 5] << 8;  // [15:8]
        stats[ic].VA *= 100;
        // VD
        stats[ic].VD  = data_b[offset];                     // [7:0]
        stats[ic].VD += (uint32_t)data_b[offset + 1] << 8;  // [15:8]
        stats[ic].VD *= 100;
        // OUV_FLAG
        memset(stats[ic].OUV_FLAG, 0, 18);
        for(int cell_offset = 0; cell_offset <= 3; cell_offset ++) {
            uint8_t mask = 0x03 << (cell_offset * 2);
            stats[ic].OUV_FLAG[0 + cell_offset] = (data_b[offset + 2] & mask) >> (cell_offset * 2);             // Cell [4:1]
            stats[ic].OUV_FLAG[4 + cell_offset] = (data_b[offset + 3] & mask) >> (cell_offset * 2);             // Cell [8:5]
            stats[ic].OUV_FLAG[8 + cell_offset] = (data_b[offset + 4] & mask) >> (cell_offset * 2);             // Cell [12:9]
            stats[ic].OUV_FLAG[12 + cell_offset] = (data_aux_d[offset + 4] & mask) >> (cell_offset * 2);        // Cell [16:13]
            if(cell_offset <= 1) {
                stats[ic].OUV_FLAG[16 + cell_offset] = (data_aux_d[offset + 5] & mask) >> (cell_offset * 2);    // Cell [18:17]
            }
        }
        // ITMP
        uint16_t temp_raw = data_a[offset + 2];         // [7:0]
        temp_raw += (uint32_t)data_a[offset + 3] << 8;  // [15:8]
        stats[ic].ITMP = temp_raw * 0.01316f - 276;
        // REV
        stats[ic].REV = (data_b[offset + 5] & 0xF0) >> 4;
        // MUXFAIL
        stats[ic].MUXFAIL = (data_b[offset + 5] & 0x02) >> 1;
        // THSD
        stats[ic].MUXFAIL = data_b[offset + 5] & 0x01;
    }
    return BMS_OK;
}



bms_status_type_t adbms1818_stats_adc_conversion(const adbms1818_handle_t *bms, adbms1818_adc_mode_t adc_mode, uint32_t wait_time) {
    wake_up(bms);
    BMS_ERR_CHECK(cmd_read(bms, ADSTATD | adc_mode, NULL, 0, false));
    // HAL_Delay(15); // 422hz All Stats needs 8.5ms, should be changed when using different speed
    HAL_Delay(wait_time);
    return BMS_OK;
}



bms_status_type_t adbms1818_gpio_adc_conversion(const adbms1818_handle_t *bms, adbms1818_adc_gpio_t adc_gpio, 
    adbms1818_adc_mode_t adc_mode, uint32_t wait_time) 
{
    wake_up(bms);
    BMS_ERR_CHECK(cmd_read(bms, ADAXD | adc_gpio | adc_mode, NULL, 0, false));
    HAL_Delay(wait_time);
    // HAL_Delay(10); // 7Khz All GPIO needs 3.9ms, should be changed when using different speed, 8ms may work
    // HAL_Delay(50);  // for 26Hz
    return BMS_OK;
}



bms_status_type_t adbms1818_cell_adc_conversion(const adbms1818_handle_t *bms, adbms1818_adc_mode_t adc_mode, uint32_t wait_time) {
    wake_up(bms);
    BMS_ERR_CHECK(cmd_read(bms, ADCV | adc_mode, NULL, 0, false));
    // HAL_Delay(20); // 422hz All Cell needs 12.8ms, should be changed when using different speed
    HAL_Delay(wait_time);
    return BMS_OK;
}



bms_status_type_t adbms1818_open_wire_adc_conversion(const adbms1818_handle_t *bms, bool pup) {
    wake_up(bms);
    BMS_ERR_CHECK(cmd_read(bms, ADOW | OPT0_26_OPT1_2K | pup << 6, NULL, 0, false));
    HAL_Delay(210); // 26hz All Cell needs 201ms, should be changed when using different speed
    return BMS_OK;
}



bms_status_type_t adbms1818_adc_handle(const adbms1818_handle_t *bms, bool blocking) {
    uint16_t tx_size = 4;
    uint8_t tx_buf[tx_size];
    bms_status_type_t ret = BMS_OK;

    wake_up(bms);
    CHIP_SELECT(bms);
    tx_buf[0] = PLADC >> 8;
    tx_buf[1] = (uint8_t)PLADC;
    apply_pec(tx_buf, 4);
    HAL_StatusTypeDef hal_stat = HAL_SPI_Transmit(bms->hspi, tx_buf, tx_size, HAL_MAX_DELAY);
    HAL_ERR_CHECK(hal_stat);
    if(blocking) {
        while(!HAL_GPIO_ReadPin(bms->sdo_port, bms->sdo_pin)) {}
        CHIP_UNSELECT(bms);
        HAL_Delay(1);
    }else if(!HAL_GPIO_ReadPin(bms->sdo_port, bms->sdo_pin)) {
        ret = BMS_BUSY;
        CHIP_UNSELECT(bms);
    }
    return ret;
}


/**
 * @param data pointer of i2c reg data (only one set of reg data needed)
 * @param i2c_real_size real bytes count of RD/WR data need i2c handle
 */
bms_status_type_t adbms1818_start_i2c(const adbms1818_handle_t *bms, uint16_t data[3], uint16_t i2c_real_size) {
    uint16_t comm_reg_size = 8 * bms->ic_count;
    uint8_t comm_reg_data[comm_reg_size];
    uint16_t st_common_size = 3 * i2c_real_size;
    uint8_t st_common_data[st_common_size];
    uint16_t offset = 0;
    
    // WRCOMM
    wake_up(bms);
    for(int ic = bms->ic_count-1; ic >= 0; ic--) {
        comm_reg_data[offset + 0] = data[0] >> 8;
        comm_reg_data[offset + 1] = data[0];
        comm_reg_data[offset + 2] = data[1] >> 8;
        comm_reg_data[offset + 3] = data[1];
        comm_reg_data[offset + 4] = data[2] >> 8;
        comm_reg_data[offset + 5] = data[2];
        apply_pec(&comm_reg_data[offset], 8);
        offset += 8;
    }
    bms_status_type_t wr_stat = cmd_write(bms, WRCOMM, comm_reg_data, comm_reg_size);
    BMS_ERR_CHECK(wr_stat);
    // STCOMM
    bms_status_type_t rd_stat = cmd_read(bms, STCOMM, st_common_data, st_common_size, false);
    BMS_ERR_CHECK(rd_stat);
    return BMS_OK;
}


/**
 * @param voltage       output, uV, [0]: G1, [1]: G2, [2]: G3, [3]: G4, [4]: G5, [5]: G6, [6]: G7, [7]: G8, [8]: G9
 * @param ref_voltage   output, uV, ref frome Aux B Register
 */
bms_status_type_t adbms1818_get_gpio_voltage(const adbms1818_handle_t *bms, uint32_t voltage[][9], uint32_t ref_voltage[]) {
    uint16_t size = bms->ic_count * 8;
    uint8_t data_a[size];
    uint8_t data_b[size];
    uint8_t data_c[size];
    uint8_t data_d[size];

    uint16_t offset = 0;
    
    wake_up(bms);
    BMS_ERR_CHECK(cmd_read(bms, RDAUXA, data_a, size, false));
    BMS_ERR_CHECK(cmd_read(bms, RDAUXB, data_b, size, false));
    BMS_ERR_CHECK(cmd_read(bms, RDAUXC, data_c, size, false));
    BMS_ERR_CHECK(cmd_read(bms, RDAUXD, data_d, size, false));

    for(int ic = 0; ic < bms->ic_count; ic++) {
        bool pec_ok_a = pec_check(&data_a[offset], 8);
        bool pec_ok_b = pec_check(&data_b[offset], 8);
        bool pec_ok_c = pec_check(&data_c[offset], 8);
        bool pec_ok_d = pec_check(&data_d[offset], 8);
        if(!pec_ok_a || !pec_ok_b || !pec_ok_c || !pec_ok_d) {
            return BMS_ERR;
        }
        voltage[ic][0] = (data_a[offset + 0] | (uint32_t)data_a[offset + 1] << 8) * 100;
        voltage[ic][1] = (data_a[offset + 2] | (uint32_t)data_a[offset + 3] << 8) * 100;
        voltage[ic][2] = (data_a[offset + 4] | (uint32_t)data_a[offset + 5] << 8) * 100;
        voltage[ic][3] = (data_b[offset + 0] | (uint32_t)data_b[offset + 1] << 8) * 100;
        voltage[ic][4] = (data_b[offset + 2] | (uint32_t)data_b[offset + 3] << 8) * 100;
        ref_voltage[ic] = (data_b[offset + 4] | (uint32_t)data_b[offset + 5] << 8) * 100;
        voltage[ic][5] = (data_c[offset + 0] | (uint32_t)data_c[offset + 1] << 8) * 100;
        voltage[ic][6] = (data_c[offset + 2] | (uint32_t)data_c[offset + 3] << 8) * 100;
        voltage[ic][7] = (data_c[offset + 4] | (uint32_t)data_c[offset + 5] << 8) * 100;
        voltage[ic][8] = (data_d[offset + 0] | (uint32_t)data_d[offset + 1] << 8) * 100;
        offset += 8;
    }
    return BMS_OK;
}



bms_status_type_t adbms1818_get_cell_voltage(const adbms1818_handle_t *bms, uint32_t voltage[][18]) {
    uint16_t size = bms->ic_count * 8;
    uint8_t data[6][size];
    uint16_t offset = 0;

    wake_up(bms);
    BMS_ERR_CHECK(cmd_read(bms, RDCVA, data[0], size, false))
    BMS_ERR_CHECK(cmd_read(bms, RDCVB, data[1], size, false))
    BMS_ERR_CHECK(cmd_read(bms, RDCVC, data[2], size, false))
    BMS_ERR_CHECK(cmd_read(bms, RDCVD, data[3], size, false))
    BMS_ERR_CHECK(cmd_read(bms, RDCVE, data[4], size, false))
    BMS_ERR_CHECK(cmd_read(bms, RDCVF, data[5], size, false))
    for(int ic = 0; ic < bms->ic_count; ic++) {
        for(int group = 0; group < 6; group++) {
            bool pec_ok = pec_check(&data[group][offset], 8);
            if(!pec_ok) {
                return BMS_ERR;
            }
            voltage[ic][group * 3 + 0] = (data[group][offset + 0] | (uint32_t)data[group][offset + 1] << 8) * 100;
            voltage[ic][group * 3 + 1] = (data[group][offset + 2] | (uint32_t)data[group][offset + 3] << 8) * 100;
            voltage[ic][group * 3 + 2] = (data[group][offset + 4] | (uint32_t)data[group][offset + 5] << 8) * 100;
        }
        offset += 8;
    }
    return BMS_OK;
}



bms_status_type_t adbms1818_open_wire_test(const adbms1818_handle_t *bms, bool success[][18]) {
    uint32_t voltage1[bms->ic_count][18];
    uint32_t voltage2[bms->ic_count][18];
    int32_t voltage_d[bms->ic_count][18];
    
    BMS_ERR_CHECK(adbms1818_open_wire_adc_conversion(bms, true));
    BMS_ERR_CHECK(adbms1818_get_cell_voltage(bms, voltage1));
    BMS_ERR_CHECK(adbms1818_open_wire_adc_conversion(bms, true));
    BMS_ERR_CHECK(adbms1818_get_cell_voltage(bms, voltage1));

    BMS_ERR_CHECK(adbms1818_open_wire_adc_conversion(bms, false));
    BMS_ERR_CHECK(adbms1818_get_cell_voltage(bms, voltage2));
    BMS_ERR_CHECK(adbms1818_open_wire_adc_conversion(bms, false));
    BMS_ERR_CHECK(adbms1818_get_cell_voltage(bms, voltage2));

    for(int ic = 0; ic < bms->ic_count; ic ++) {
        for(int cell = 0; cell < 18; cell ++) {
            voltage_d[ic][cell] = (int32_t)voltage1[ic][cell] - (int32_t)voltage2[ic][cell];
            if(cell == 0 && voltage_d[ic][cell] == 0) {
                success[ic][cell] = false;
            }else if(cell != 0 && voltage_d[ic][cell] < -400000) {
                success[ic][cell] = false;
            }else {
                success[ic][cell] = true;
            }
        }
    }
    return BMS_OK;
}



