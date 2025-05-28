#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdbool.h>
#include <stdint.h>

#define IC_COUNT 1

// #define DCC_ENABLE //define to enable cell discharge

/* 
    avail = 1
    cell0 = LSB
    01 1111 0111 1101 1111 (cell 5, 11, 17 not avail)
*/
#define CELL_MASK   0x1F7DF
#define CELL_COUNT  (15 * IC_COUNT)

/*
    avail = 1
    ic0 = index0, ic1 = index1 ...
    th0 = LSB
*/
#define THERMISTORS_MASK {\
    0xFFFFFFFF,\
    0xFFFFFFFF,\
    0x00FFFFFF,\
    0x00FFFFFF,\
    0xFFFFFFFF,\
    0xFFFFFFFF,\
    0xFFFFFFFF\
} 

#define ERROR_TIMEOUT   3000 // ms

#define MAX_LOG_COUNT       2
#define MAX_SIZE_PER_LOG    300

#define BALANCING_DIFF_THRES        10000       // uV, discharge when (voltage - avg_voltage) > this_value
#define FULL_CHARGE_VOLTAGE         4140000     // uV
#define CELL_OVER_VOLTAGE_THRES     4200000     // uV
#define CELL_UNDER_VOLTAGE_THRES    3200000     // uV

#define THERMISTOR_OVER_TEMPERATURE_THRES           55      // Celsius
#define THERMISTOR_UNDER_TEMPERATURE_THRES          -10     // Celsius
#define THERMISTOR_UNDER_TEMPERATURE_COUNT_THRES    15      // per IC

#define PARALLEL_FAIL_MIN_V_DROP            10000       // uV, parallel_fail detect active at avg v_drop > this_value
#define PARALLEL_FAIL_V_DROP_THRES_RATIO    1.8f        // uV, parallel_fail when v_drop > (avg_drop * this_value)
#define PARALLEL_FAIL_SAMPLE_INTERVAL       10000       // ms

#define CAN_DEBUGGER


enum {
    UART_INFO_MODE,
    UART_LOG_MODE,
};

/* Global Ctrl */
extern uint8_t uart_debug_mode;
extern volatile bool uart2_tx_dma_cplt;

#endif