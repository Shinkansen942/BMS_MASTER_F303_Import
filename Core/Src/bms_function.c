#include <stdlib.h>
#include "bms_function.h"
#include "string.h"
#include "adbms1818.h"

const uint16_t thermistor_table[] = { // -13 ~ 80, step 1
    65091,  61596,  58310,  55218,  52308,  49569,  46989,  44559,
    42268,  40108,  38071,  36150,  34336,  32624,  31007,  29480,
    28036,  26672,  25381,  24161,  23006,  21912,  20877,  19897,
    18968,  18088,  17253,  16462,  15711,  14999,  14323,  13681,
    13072,  12493,  11943,  11420,  10923,  10450,  10000,  9572,
    9165,   8777,   8408,   8056,   7721,   7401,   7097,   6807,
    6530,   6266,   6014,   5773,   5543,   5324,   5114,   4914,
    4723,   4540,   4365,   4198,   4038,   3885,   3739,   3599,
    3465,   3336,   3213,   3095,   2982,   2874,   2770,   2671,
    2575,   2484,   2396,   2312,   2231,   2153,   2079,   2007,
    1938,   1872,   1809,   1748,   1689,   1633,   1578,   1526,
    1476,   1428,   1381,   1336,   1293,   1252
};

uint32_t gpio_voltage[IC_COUNT][9];
uint32_t ref_voltage[IC_COUNT];

void update_temperature(const adbms1818_handle_t *bms, float temperature[IC_COUNT][32], adbms1818_cfg_t *cfg) {
    // static uint8_t mux_id;
    static uint8_t mux_ctrl_shift;

    // uint32_t gpio_voltage[IC_COUNT][4];
    // uint32_t ref_voltage[IC_COUNT];

    // uint8_t mux_ctrl = 1 << mux_ctrl_shift;
    // uint16_t i2c_data[3];
    // uint8_t mux_addr = (0x4C | mux_id) << 1;
    
    // // setup i2c mux
    // i2c_data[0] = I2C_DATA(I2C_START, mux_addr, I2C_M_NAK);
    // i2c_data[1] = I2C_DATA(I2C_BLANK, mux_ctrl, I2C_M_NAK_STOP);
    // i2c_data[3] = 0;
    // adbms1818_start_i2c(bms, i2c_data, 2);

    // setup basic mux
    cfg[0].GPIO = mux_ctrl_shift << 6|0x01F;
    for (size_t i = 1; i < IC_COUNT; i++)
    {
        cfg[i].GPIO = cfg[0].GPIO;
    }
    adbms1818_write_cfg(bms,cfg);

    HAL_Delay(1); // wait mux switch and capacitor charge
    // adc conversion
    adbms1818_gpio_adc_conversion(bms, ADC_GPIO_ALL, OPT0_422_OPT1_1K, 0);
    uint32_t conversion_start = HAL_GetTick();
    while(HAL_GetTick() - conversion_start < 30) {
        loop_no_adbms();
        
    }
    adbms1818_get_gpio_voltage(bms, gpio_voltage, ref_voltage);
    // temperature calculate
    for(int ic = 0; ic < IC_COUNT; ic++) {
        // 3V3 -- Rs -(ADC)- Th -- GND
        const uint32_t r_s = 10000; 
        uint16_t ref_voltage_mv = ref_voltage[ic] * 0.001f; // uV to mV

        for (size_t it = 0; it < 4; it++)
        {
            volatile uint16_t adc_voltage = (uint16_t)((float)gpio_voltage[ic][it] * 0.001f); // uV to mV
            /*  
                formula: 
                th  = thermistor resistance
                rs  = up series-restistor resistance
                adc = adc read voltage
                    3.3 * th / (th + rs) = adc
                    3.3 * th = adc * th + adc * rs
                    (3.3 - adc) * th = adc * rs
                    th = adc * rs / (3.3 - adc)
            */
            // ADC voltage to thermistor resistance
            float ref_diff = ref_voltage_mv - adc_voltage;
            if (ref_diff < 0)
            {
                ref_diff = 0;
            }
            volatile float r_th = (float)(adc_voltage * r_s) / ref_diff;
            // get temperature from table
            uint16_t thermistor_table_count = sizeof(thermistor_table) / sizeof(thermistor_table[0]);
            for(int i = 0; i < thermistor_table_count; i ++) {
                // thermistor_table first
                if(r_th >= thermistor_table[0]) {
                    temperature[ic][it * 8 + mux_ctrl_shift] = -13;
                    break;

                // thermistor_table end
                }else if(r_th <= thermistor_table[thermistor_table_count - 1]) {
                    temperature[ic][it * 8 + mux_ctrl_shift] = (float)thermistor_table_count - 13 - 1;
                    break;
            
                // find thermistor_table < r_th
                }else if(thermistor_table[i] < r_th) {
                    float r_th_per_c = thermistor_table[i+1] - thermistor_table[i];
                    temperature[ic][it * 8 + mux_ctrl_shift] = (float)i - 13 + (r_th - thermistor_table[i]) / r_th_per_c;
                    break;
                }
            }
        }
    
        // /*  
        //     formula: 
        //     th  = thermistor resistance
        //     rs  = up series-restistor resistance
        //     adc = adc read voltage
        //         3.3 * th / (th + rs) = adc
        //         3.3 * th = adc * th + adc * rs
        //         (3.3 - adc) * th = adc * rs
        //         th = adc * rs / (3.3 - adc)
        // */
        // // ADC voltage to thermistor resistance
        // float r_th = (float)(adc_voltage * r_s) / (float)(ref_voltage_mv - adc_voltage);
        // // get temperature from table
        // uint16_t thermistor_table_count = sizeof(thermistor_table) / sizeof(thermistor_table[0]);
        // for(int i = 0; i < thermistor_table_count; i ++) {
        //     // thermistor_table first
        //     if(r_th >= thermistor_table[0]) {
        //         temperature[ic][mux_id * 8 + mux_ctrl_shift] = -13;
        //         break;

        //     // thermistor_table end
        //     }else if(r_th <= thermistor_table[thermistor_table_count - 1]) {
        //         temperature[ic][mux_id * 8 + mux_ctrl_shift] = (float)thermistor_table_count - 13 - 1;
        //         break;
            
        //     // find thermistor_table < r_th
        //     }else if(thermistor_table[i] < r_th) {
        //         float r_th_per_c = thermistor_table[i+1] - thermistor_table[i];
        //         temperature[ic][mux_id * 8 + mux_ctrl_shift] = (float)i - 13 + (r_th - thermistor_table[i]) / r_th_per_c;
        //         break;
        //     }
        // }
    }
    // // clear the mux at last ctrl shift
    // if(mux_ctrl_shift == 7) {
    //     mux_ctrl = 0;
    //     i2c_data[0] = I2C_DATA(I2C_START, mux_addr, I2C_M_NAK);
    //     i2c_data[1] = I2C_DATA(I2C_BLANK, mux_ctrl, I2C_M_NAK_STOP);
    //     i2c_data[3] = 0;
    //     adbms1818_start_i2c(bms, i2c_data, 2);
    // }

    // move mux shift
    if(++ mux_ctrl_shift >= 8) {
        mux_ctrl_shift = 0;
        // move mux ic idx
        // if(++ mux_id >= 4) {
        //     mux_id = 0;
        // }
    }
    
}



void measure_current(ADC_HandleTypeDef *hadc, uint16_t raw_value, int32_t *current) {
    static uint32_t last_current_measure;
    static bool cal_finish = false;
    static uint16_t cal_count = 0;
    static uint32_t zero_current_offset;    // adc value
    const int32_t adc_to_micro_volt = 3300000 / 4095;
    const int32_t micro_volt_to_milli_amp = 180.18e-3f;    // A/mV or mA/uV
    
    if(HAL_GetTick() - last_current_measure >= 10) {
        uint32_t dt = HAL_GetTick() - last_current_measure;
        last_current_measure = HAL_GetTick();

        // calibration
        if(!cal_finish) {
            zero_current_offset += raw_value;
            if(++ cal_count >= 100) {
                cal_finish = true;
                zero_current_offset /= cal_count;
            }
            return;
        }
        int16_t center_raw = raw_value - zero_current_offset;
        int32_t current_voltage = adc_to_micro_volt * center_raw;    // adc value to uV
        *current = current_voltage * micro_volt_to_milli_amp; // uV to mA
        // *capacity += *current * dt; // mA,ms to uAs
    }
}



void get_balancing_control_config(
    const adbms1818_handle_t *bms, 
    adbms1818_cfg_t control_cfg[IC_COUNT],
    const uint32_t cell_voltage[IC_COUNT][18],
    uint32_t avg_cell_voltage
) {
    for(int ic = 0; ic < IC_COUNT; ic++) {
        uint32_t DCC = 0;
        for(int cell = 0; cell < 18; cell++) {
            // non mask, skip
            if(!(1 << cell & CELL_MASK)) {
                continue;
            }
            // balancing
            if((int32_t)cell_voltage[ic][cell] - (int32_t)avg_cell_voltage > BALANCING_DIFF_THRES) {
                DCC |= 1 << (cell + 1);
            }
        }
        control_cfg[ic].DCC = DCC;
    }
}



void open_wire_error_detect(report_handle_t *report_handle, const adbms1818_handle_t *bms) {
    bool open_wire_success[IC_COUNT][18];
    adbms1818_open_wire_test(bms, open_wire_success);
    for(int ic = 0; ic < IC_COUNT; ic ++) {
        for(int cell = 0; cell < 18; cell ++) {
            // non mask, skip
            if(!(1 << cell & CELL_MASK)) {
                continue;
            }
            if(!open_wire_success[ic][cell]) {
                report_add(report_handle, ic, CELL_OPEN_WIRE_FAILED, cell, REPORT_PERSIST);    // update report
            }
        }
    }
}



void parallel_error_detect(report_handle_t *report_handle, const uint32_t cell_voltage[IC_COUNT][18]) {
    static uint32_t last_sample;
    static uint32_t last_cell_voltage[IC_COUNT][18];
    static bool first_sample = true;

    if(HAL_GetTick() - last_sample >= PARALLEL_FAIL_SAMPLE_INTERVAL) {
        int32_t delta[IC_COUNT][18];
        int32_t avg_delta = 0;
        last_sample = HAL_GetTick();
        // calculate delta_voltage and avg delta_voltage
        for(int ic = 0; ic < IC_COUNT; ic++) {
            // clear old node before update it
            report_clear_type(report_handle, ic, CELL_PARALLEL_FAILED, EMPTY_TYPE, EMPTY_TYPE);
            for(int cell = 0; cell < 18; cell++) {
                // non mask, skip
                if(!(1 << cell & CELL_MASK)) {
                    continue;
                }
                delta[ic][cell] = (int32_t)cell_voltage[ic][cell] - (int32_t)last_cell_voltage[ic][cell];
                avg_delta += delta[ic][cell];
                last_cell_voltage[ic][cell] = cell_voltage[ic][cell];
            }
        }
        // first sample
        if(first_sample) {
            first_sample = false;
            return;
        }
        // avg_delta = 0
        if(!avg_delta) {
            return;
        }
        avg_delta /= CELL_COUNT;
        // min v_drop requirement, charging should not pass this line
        if(avg_delta > -abs(PARALLEL_FAIL_MIN_V_DROP)) {
            return;
        }
        // avg delta to threshold
        int32_t threshold = avg_delta * PARALLEL_FAIL_V_DROP_THRES_RATIO;
        // parallel fail check
        for(int ic = 0; ic < IC_COUNT; ic++) {
            for(int cell = 0; cell < 18; cell++) { 
                // non mask, skip
                if(!(1 << cell & CELL_MASK)) {
                    continue;
                }
                if(delta[ic][cell] < threshold) {
                    report_add(report_handle, ic, CELL_PARALLEL_FAILED, cell, REPORT_VARIANT);
                }
            }
        }
    }
}



void cell_voltage_observe(report_handle_t *report_handle,
    const uint32_t cell_voltage[IC_COUNT][18],
    const adbms1818_stats_t stats[IC_COUNT]
) {
    report_handle->info |= 0x01;    // set battery full
    for(int ic = 0; ic < IC_COUNT; ic ++) {
        // clear old node before update it
        report_clear_type(report_handle, ic, CELL_OV_UV, EMPTY_TYPE, EMPTY_TYPE);
        for(int cell = 0; cell < 18; cell ++) {
            // non mask, skip
            if(!(1 << cell & CELL_MASK)) {
                continue;
            }
            // cell not full detect
            if(cell_voltage[ic][cell] < FULL_CHARGE_VOLTAGE) {
                report_handle->info &= 0xFE;    // clear battery full
            }
            // ov uv detect
            if(stats[ic].OUV_FLAG[cell] != 0) {
                report_add(report_handle, ic, CELL_OV_UV, cell, REPORT_VARIANT);    // update report
            }
        }
    }
}



void temperature_error_detect(report_handle_t *report_handle, const float temperature[IC_COUNT][32]) {
    // mask
    uint32_t thermistors_mask[] = THERMISTORS_MASK;
    for(int ic = 0; ic < IC_COUNT; ic ++) {
        // clear old node before update it
        report_clear_type(report_handle, ic, THERMISTOR_OT, THERMISTOR_UT, EMPTY_TYPE);
        // thermistor
        for(int th_idx = 0; th_idx < 32; th_idx ++) {
            // non mask, skip
            if(!(1 << th_idx & thermistors_mask[ic])) {
                continue;
            }
            // temperature out of range detect
            if(temperature[ic][th_idx] > THERMISTOR_OVER_TEMPERATURE_THRES && temperature[ic][th_idx] < 75) {
                report_add(report_handle, ic, THERMISTOR_OT, th_idx, REPORT_VARIANT);    // update report
            }else if(temperature[ic][th_idx] < THERMISTOR_UNDER_TEMPERATURE_THRES) {
                report_add(report_handle, ic, THERMISTOR_UT, th_idx, REPORT_VARIANT);    // update report
            }
        }
    }
}



bool is_error(report_handle_t *report_handle) {
    bool error_output = false;

    // count error
    report_update_error_count(report_handle);

    // voltage error
    if(report_handle->err_count[0]) {
        error_output = true;
    }
    // temperature error
    // thermistor
    for(int ic = 0; ic < IC_COUNT; ic ++) {
        // thermistor over temperature
        int th_ot_count = report_get_type_count(report_handle, ic, THERMISTOR_OT, EMPTY_TYPE, EMPTY_TYPE);
        if(th_ot_count) {
            error_output = true;
            break;
        }
        // thermistor under temperature
        int th_ut_count = report_get_type_count(report_handle, ic, THERMISTOR_UT, EMPTY_TYPE, EMPTY_TYPE);
        if(th_ut_count > THERMISTOR_UNDER_TEMPERATURE_COUNT_THRES) {
            error_output = true;
            break;
        }
    }
    // full charge
    if(report_handle->info & 0x01) {
        error_output = true;
    }
    
    // error output report
    report_handle->info &= 0xFD;                // clear
    report_handle->info |= error_output << 1;   // update
    
    return error_output;
}



static TIM_HandleTypeDef *error_htim;

void start_error_timer(TIM_HandleTypeDef *htim) {
    error_htim = htim;
    HAL_TIM_Base_Start_IT(htim);
    __HAL_TIM_SET_AUTORELOAD(htim, ERROR_TIMEOUT * 2); // 0.5 ms/tick
}


/**
 * @param error 0: clear error timer counter, set pin no error
 */
void update_error_timer(bool error) {
    if(!error) {
        __HAL_TIM_SET_COUNTER (error_htim, 0);
        HAL_GPIO_WritePin(ERR_GPIO_Port, ERR_Pin, SET);
    }   
}



/** Call this when error timer timeout
 * @param htim interrupted htim
 */
void error_timer_it_handle(TIM_HandleTypeDef *htim) {
    if(htim->Instance == error_htim->Instance) {
        HAL_GPIO_WritePin(ERR_GPIO_Port, ERR_Pin, RESET);
    }
}
