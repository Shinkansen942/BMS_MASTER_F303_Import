#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "can.h"
#include "usart.h"
#include "tim.h"
#include "adc.h"

#include "bms_function.h"

/**
 * @param min_wait ms
 */
#define WAIT_ADBMS_CONVERSION(min_wait) conversion_start = HAL_GetTick();\
    do{ loop_no_adbms(); }while(HAL_GetTick() - conversion_start < (min_wait))

/* Global Ctrl */
volatile bool uart2_tx_dma_cplt = true;
uint8_t uart_debug_mode = UART_INFO_MODE;

/* Variables */
static adbms1818_handle_t bms;
static adbms1818_cfg_t bms_cfg[IC_COUNT];
static CAN_TxHeaderTypeDef can_tx_header;
static report_handle_t report_handle;

static char cmd_buffer[30] = {0};
static uint8_t log_buffer[MAX_LOG_COUNT][MAX_SIZE_PER_LOG] = {0};
static uint8_t log_cursor = 0;
volatile static bool error_logged = false;
volatile static bool cmd_flag;  // true: new cmd

static uint16_t current_sense_raw[2] = {0};



void init() {
    /* ADC */
    HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)current_sense_raw, 2);

    /* adbms1818 */
    // init handler
    adbms1818_handle_init(
        &bms, &hspi1, 
        SPI1_CS_GPIO_Port, SPI1_CS_Pin, 
        ADBMS1818_2_MISO_GPIO_Port, ADBMS1818_2_MISO_Pin,
        IC_COUNT
    );
    // config
    adbms1818_get_default_cfg(&bms, bms_cfg);
    bms_cfg[0].DCTO = DCTO_1_MIN;
    bms_cfg[0].VOV = CELL_OVER_VOLTAGE_THRES;
    bms_cfg[0].VUV = CELL_UNDER_VOLTAGE_THRES;
    for(int ic = 0; ic < IC_COUNT; ic ++) {
        bms_cfg[ic] = bms_cfg[0];
    }
    adbms1818_write_cfg(&bms, bms_cfg);
    // error report init
    report_init(&report_handle);

    /* CAN */
    // transceiever pin init
    HAL_GPIO_WritePin(CAN_EN_GPIO_Port, CAN_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CAN_S_GPIO_Port, CAN_S_Pin, GPIO_PIN_RESET);
    // config
    can_tx_header.IDE = CAN_ID_STD;
    can_tx_header.RTR = CAN_RTR_DATA;
    can_tx_header.StdId = 0x87;
    can_tx_header.DLC = 8;
    // can start
    HAL_CAN_Start(&hcan);

    /* Start */
    // make adbms1818 REF_ON
    adbms1818_stats_adc_conversion(&bms, OPT0_422_OPT1_1K, 20);
    HAL_Delay(10);

    /* Self Test */
    // open wire test
    open_wire_error_detect(&report_handle, &bms);

    report_update_error_count(&report_handle);
    start_error_timer(&htim3);

    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t*)cmd_buffer, sizeof(cmd_buffer));
}



static adbms1818_stats_t    stats[IC_COUNT];
static uint32_t             cell_voltage[IC_COUNT][18]; // uV
static float                temperature[IC_COUNT][32];  // C
static int32_t              current;        // mA
static int64_t              capacity;       // uA-s
static uint32_t             total_voltage;  // uA
static uint32_t             avg_cell_voltage;   // uA


static uint32_t _debug_loop_no_adbms_dt;

void loop_no_adbms() {
    uint32_t start_time = HAL_GetTick();
    /* Update BMS Variable */
    // total voltage
    total_voltage = 0;
    for(int ic = 0; ic < IC_COUNT; ic ++) {
        total_voltage += stats[ic].SC;
    }
    // avg cell voltage
    avg_cell_voltage = total_voltage / CELL_COUNT;

    /* Report */
    can_bus_report_handle(&hcan, &can_tx_header, &report_handle, cell_voltage, temperature, stats, 
        current, capacity, total_voltage, avg_cell_voltage);
    uart_report_handle(1000, &report_handle, cell_voltage, temperature, stats);

    /* Current Measure */
    measure_current(&hadc1, current_sense_raw[0], &current);

    /* Error Detect */
    // parallel_error_detect(&report_handle, cell_voltage);
    // temperature_error_detect(&report_handle, temperature);
    cell_voltage_observe(&report_handle, cell_voltage, stats);

    /* Balancing */
    #ifdef DCC_ENABLE
    get_balancing_control_config(&bms, bms_cfg, cell_voltage, avg_cell_voltage);
    #endif

    /* Error handle */
    bool error_state = is_error(&report_handle);
    update_error_timer(error_state);
    if(!error_state) {
        error_logged = false;
    }
    
    // dt
    _debug_loop_no_adbms_dt = HAL_GetTick() - start_time;
}



static uint32_t _debug_loop_dt;

volatile void loop() {
    uint32_t start_time = HAL_GetTick();
    /* Schedule Variable */
    static uint32_t last_voltage_measure, last_bms_ctrl;
    static uint32_t conversion_start;

    /* Measure temperature */
    update_temperature(&bms, temperature, bms_cfg);

    /* Measure cell, stats voltage */
    if(HAL_GetTick() - last_voltage_measure >= 500) {
        last_voltage_measure = HAL_GetTick();
        // Cell Voltage
        adbms1818_cell_adc_conversion(&bms, OPT0_422_OPT1_1K, 0);
        WAIT_ADBMS_CONVERSION(20);
        adbms1818_get_cell_voltage(&bms, cell_voltage);
        // Stats
        adbms1818_stats_adc_conversion(&bms, OPT0_422_OPT1_1K, 0);
        WAIT_ADBMS_CONVERSION(15);
        adbms1818_read_stats(&bms, stats);

        HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
    }

    // /* BMS Control */
    // if(HAL_GetTick() - last_bms_ctrl >= 500) {
    //     last_bms_ctrl = HAL_GetTick();
    //     adbms1818_write_cfg(&bms, bms_cfg);
    // }
    /* loop no adbms */
    loop_no_adbms();
    if(cmd_flag) {
        cmd_flag = false;
        cmd_process(cmd_buffer, log_buffer);
    }

    // dt
    _debug_loop_dt = HAL_GetTick() - start_time;
}



/* Error Output Timer */
void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef *htim) {
    error_timer_it_handle(htim);
    /* Log */
    // need log
    if(!error_logged) {
        report_create_log(&report_handle, log_buffer[log_cursor ++]);
        error_logged = true;
    }
    // log cursor
    if(log_cursor >= MAX_LOG_COUNT) {
        log_cursor = 0;
    }
}



void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
    if(huart->Instance == USART2) {
        cmd_flag = true;
    }
}



void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if(huart->Instance == huart1.Instance) {
        uart2_tx_dma_cplt = true;
    }
}