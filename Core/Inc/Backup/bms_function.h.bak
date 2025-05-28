#ifndef __BMS_FUNCTION_H__
#define __BMS_FUNCTION_H__

#include "report.h"
#include "tim.h"


void update_temperature(const adbms1818_handle_t *bms, float temperature[IC_COUNT][32], adbms1818_cfg_t *cfg);


void get_balancing_control_config(
    const adbms1818_handle_t *bms, 
    adbms1818_cfg_t control_cfg[IC_COUNT],
    const uint32_t cell_voltage[IC_COUNT][18],
    uint32_t avg_cell_voltage
);


void measure_current(ADC_HandleTypeDef *hadc, uint16_t raw_value, int32_t *current);


void open_wire_error_detect(report_handle_t *report_handle, const adbms1818_handle_t *bms);


void parallel_error_detect(report_handle_t *report_handle, const uint32_t cell_voltage[IC_COUNT][18]);


void cell_voltage_observe(report_handle_t *report_handle,
    const uint32_t cell_voltage[IC_COUNT][18],
    const adbms1818_stats_t stats[IC_COUNT]
);


void temperature_error_detect(report_handle_t *report_handle, const float temperature[IC_COUNT][32]);


bool is_error(report_handle_t *report_handle);


void start_error_timer(TIM_HandleTypeDef *htim);


void update_error_timer(bool error);


void error_timer_it_handle(TIM_HandleTypeDef *htim);


extern void loop_no_adbms();


#endif