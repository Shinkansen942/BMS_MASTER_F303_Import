#ifndef __REPORT_H__
#define __REPORT_H__


#include "config.h"
#include "can.h"
#include "adbms1818.h"


#define REPORT_DETAIL_HEAD(rp_handle, ic) (&((rp_handle)->detail[(ic)]))
#define REPORT_DETAIL_FIRST(rp_handle, ic) ((REPORT_DETAIL_HEAD(rp_handle, ic))->next)


typedef enum {
    REPORT_VARIANT  = 0x00,
    REPORT_PERSIST  = 0x01
}report_flag_t;


typedef enum {
    EMPTY_TYPE              = 0,
    CELL_OPEN_WIRE_FAILED   = 1,
    CELL_OV_UV              = 2,
    CELL_PARALLEL_FAILED    = 3,
    THERMISTOR_OT           = 11,
    THERMISTOR_UT           = 12,
    IC_OT                   = 13
}report_type_t;


typedef struct report_detail_s{
    /*
        1   : cell open wire failed
        2   : cell ov uv
        3   : cell parallel failed
        11  : cell ot ut
        12  : ic ot
    */
    report_type_t type;
    int value;
    report_flag_t flag;
    struct report_detail_s *prev;
    struct report_detail_s *next;
}report_detail_t;


typedef struct {
    /*
        bit 0   : battery full
        bit 1   : error output
    */
    uint8_t info;
    /*
        voltage error:
            cell open wire check failed, cell ov uv, cell parallel failed
        temperature error:
            cell ot, ut, ic ot
    */
    uint8_t err_count[5];
    report_detail_t detail[IC_COUNT];
}report_handle_t;



void report_init(report_handle_t *report_handle);


void report_add(report_handle_t *report_handle, uint8_t ic, report_type_t type, int value, report_flag_t flag);


void report_clear_variant(report_handle_t *report_handle, uint8_t ic);


void report_update_error_count(report_handle_t *report_handle);


void report_clear_type(report_handle_t *report_handle, uint8_t ic, uint8_t type1, uint8_t type2, uint8_t type3);


int report_get_type_count(report_handle_t *report_handle, uint8_t ic, report_type_t type1, report_type_t type2, report_type_t type3);


bool report_create_log(report_handle_t *report_handle, uint8_t *log_output);


void print_log(const uint8_t *log, UART_HandleTypeDef *huart);


void cmd_process(char *cmd_buffer, uint8_t log_buffer[][MAX_SIZE_PER_LOG]);


void can_bus_report_handle(
    CAN_HandleTypeDef *hcan, 
    CAN_TxHeaderTypeDef *can_tx_header, 
    report_handle_t *report_handle,
    const uint32_t cell_voltage[IC_COUNT][18], 
    const float temperature[IC_COUNT][32], 
    const adbms1818_stats_t stats[IC_COUNT],
    int32_t current, 
    int64_t capacity,
    uint32_t total_voltage,
    uint32_t avg_cell_voltage
);

void uart_report_handle(
    uint32_t report_interval,
    report_handle_t *report_handle,
    const uint32_t cell_voltage[IC_COUNT][18], 
    const float temperature[IC_COUNT][32], 
    const adbms1818_stats_t stats[IC_COUNT]
);



#endif