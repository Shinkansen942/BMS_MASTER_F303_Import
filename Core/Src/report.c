#include "report.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "usart.h"
#include "soc.h"

/*========== report detail data struct ===========*/

/*
    node head
     ||
     \/
    node1 <=> node2 <=> node3
*/

#define LOG_FULL_CHECK(current) do{if(current >= MAX_SIZE_PER_LOG){return false;}}while(0);



void rd_init(report_detail_t *rd_head) {
    rd_head->flag = 0;
    rd_head->type = 0;
    rd_head->value = 0;
    rd_head->next = NULL;
    rd_head->prev = NULL;
}



report_detail_t* rd_append_node(report_detail_t *rd_head) {
    report_detail_t *rd_new = malloc(sizeof(*rd_head)); // alloc new node memory
    // cannot alloc memory
    if(!rd_new) {
        return NULL;
    }
    // setup node
    report_detail_t *rd_origin_next = rd_head->next;
    if(rd_origin_next) { // data exist
        rd_origin_next->prev = rd_new;
    }
    rd_head->next = rd_new;
    rd_new->next = rd_origin_next;
    rd_new->prev = rd_head;
    return rd_new;
}



report_detail_t* rd_remove_node(report_detail_t *rd) {
    // cannot alloc memory
    if(!rd) {
        return NULL;
    }
    // setup node
    report_detail_t* rd_next = rd->next;
    rd->prev->next = rd_next;
    if(rd_next) {
        rd_next->prev = rd->prev;
    }
    free(rd);
    return rd_next;
}



/*========== report handle function ===========*/

void report_init(report_handle_t *report_handle) {
    report_handle->info = 0;
    memset(report_handle->err_count, 0, sizeof(report_handle->err_count));
    for(int ic = 0; ic < IC_COUNT; ic++) {
        rd_init(&(report_handle->detail[ic]));
    }
}



void report_add(report_handle_t *report_handle, uint8_t ic, report_type_t type, int value, report_flag_t flag) {
    report_detail_t* rd = rd_append_node(REPORT_DETAIL_HEAD(report_handle, ic));
    // append node failed
    if(!rd) {
        return;
    }
    // setup new report detail
    rd->type = type;
    rd->value = value;
    rd->flag = flag;
}



void report_clear_variant(report_handle_t *report_handle, uint8_t ic) {
    report_detail_t* rd = REPORT_DETAIL_FIRST(report_handle, ic);
    while(rd) { // run when rd avail
        if(!(rd->flag & REPORT_PERSIST)) { // variant
            rd = rd_remove_node(rd);
        }else {
            rd = rd->next;
        }
    }
}



void report_update_error_count(report_handle_t *report_handle) {
    uint8_t voltage_error_count = 0;
    uint8_t temperature_error_count = 0;
    // count error
    for(int ic = 0; ic < IC_COUNT; ic ++) {
        report_detail_t *rd = REPORT_DETAIL_FIRST(report_handle, ic);
        while(rd) {
            // voltage error
            if(rd->type == CELL_OPEN_WIRE_FAILED
                || rd->type == CELL_OV_UV
                || rd->type == CELL_PARALLEL_FAILED
            ) {
                voltage_error_count ++;
            }
            // temperature error
            if(rd->type == THERMISTOR_OT
                || rd->type == THERMISTOR_UT
                || rd->type == IC_OT
            ) {
                temperature_error_count ++;
            }
            // go to next rd
            rd = rd->next;
        }
    }
    // write to handle
    report_handle->err_count[0] = voltage_error_count;
    report_handle->err_count[1] = temperature_error_count;
}



void report_clear_type(report_handle_t *report_handle, uint8_t ic, uint8_t type1, uint8_t type2, uint8_t type3) {
    report_detail_t* rd = REPORT_DETAIL_FIRST(report_handle, ic);
    while(rd) { // run when rd avail
        if(rd->type == type1 || rd->type == type2 || rd->type == type3) {
            rd = rd_remove_node(rd);
        }else {
            rd = rd->next;
        }
    }
}



int report_get_type_count(report_handle_t *report_handle, uint8_t ic, report_type_t type1, report_type_t type2, report_type_t type3) {
    report_detail_t* rd = REPORT_DETAIL_FIRST(report_handle, ic);
    int count = 0;
    while(rd) { // run when rd avail
        if(rd->type == type1 || rd->type == type2 || rd->type == type3) {
            count ++;
        }
        rd = rd->next;
    }
    return count;
}



bool report_create_log(report_handle_t *report_handle, uint8_t *log_output) {
    memset(log_output, 0, MAX_SIZE_PER_LOG);
    // timestamp
    uint32_t *timestamp = (uint32_t*)log_output;
    *timestamp = HAL_GetTick();
    // detail
    uint32_t cursor = 4;
    for(int ic = 0; ic < IC_COUNT; ic ++) {
        uint8_t *log_count = &(log_output[cursor ++]);
        LOG_FULL_CHECK(cursor);
        report_detail_t *rd = REPORT_DETAIL_FIRST(report_handle, ic);
        while(rd) {
            log_output[cursor ++] = rd->type;
            LOG_FULL_CHECK(cursor);
            log_output[cursor ++] = rd->value;
            LOG_FULL_CHECK(cursor);
            (*log_count) ++;
            // go to next rd
            rd = rd->next;
        }
    }
    return true;
}



void print_log(const uint8_t *log, UART_HandleTypeDef *huart) {
    const uint32_t *timestamp = (uint32_t*)log;
    // print timestamp
    char msg_buf[100] = {0};
    sprintf(msg_buf, "Timestamp:%010lu\n", *timestamp);
    HAL_UART_Transmit(huart, (uint8_t*)msg_buf, strlen(msg_buf), 10);
    memset(msg_buf, 0, sizeof(msg_buf));
    // print detail
    uint32_t cursor = 4;
    for(int ic = 0; ic < IC_COUNT; ic ++) {
        const uint8_t *log_count = &(log[cursor ++]);
        sprintf(msg_buf, "\tSegment: %02u, Report Count: %03u\n", ic, *log_count);
        HAL_UART_Transmit(huart, (uint8_t*)msg_buf, strlen(msg_buf), 10);
        memset(msg_buf, 0, sizeof(msg_buf));
        for(uint8_t log_idx = 0; log_idx < *log_count; log_idx ++) {
            uint8_t type = log[cursor++];
            uint8_t value = log[cursor++];
            switch(type) {
            case CELL_OPEN_WIRE_FAILED:
                sprintf(msg_buf, "\t\tOpen Wire Failed: %u\n", value);
                break;
            
            case CELL_OV_UV:
                sprintf(msg_buf, "\t\tOV UV Failed: %u\n", value);
                break;

            case CELL_PARALLEL_FAILED:
                sprintf(msg_buf, "\t\tParallel Failed: %u\n", value);
                break;

            case THERMISTOR_OT:
                sprintf(msg_buf, "\t\tThermistor OT Failed: %u\n", value);
                break;

            case THERMISTOR_UT:
                sprintf(msg_buf, "\t\tThermistor UT Failed: %u\n", value);
                break;

            default:
                sprintf(msg_buf, "\t\tUnknown Failed: %u %u\n", type, value);
            }
            HAL_UART_Transmit(huart, (uint8_t*)msg_buf, strlen(msg_buf), 10);
            memset(msg_buf, 0, sizeof(msg_buf));
        }
    }
}



void cmd_process(char *cmd_buffer, uint8_t log_buffer[][MAX_SIZE_PER_LOG]) {
    char msg_buffer[250] = {0};
    /* CMD */
    // current timestamp
    if(strncmp(cmd_buffer, "CT", 2) == 0) {
        sprintf(msg_buffer, "current timestamp: %lu\n", HAL_GetTick());
        HAL_UART_Transmit(&huart1, (uint8_t*)msg_buffer, strlen(msg_buffer), 10);

    // print log
    }else if(strncmp(cmd_buffer, "PL", 2) == 0) {
        sprintf(msg_buffer, "print log: \n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg_buffer, strlen(msg_buffer), 10);
        for(uint8_t log_idx = 0; log_idx < MAX_LOG_COUNT; log_idx ++) {
            print_log(log_buffer[log_idx], &huart1);
        }
    // uart log mode
    }else if(strncmp(cmd_buffer, "UL", 2) == 0) {
        uart_debug_mode = UART_LOG_MODE;

    // uart info mode
    }else if(strncmp(cmd_buffer, "UI", 2) == 0) {
        uart_debug_mode = UART_INFO_MODE;
    
    // undefine command
    }else {
        sprintf(msg_buffer, "好色喔翁健展\n\
            CT: Current Timestamp\n\
            PL: Print Log\n\
            UL: Uart Log Mode\n\
            UI: Uart Info Mode\n\
        ");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg_buffer, strlen(msg_buffer), 10);
    }
    memset(cmd_buffer, 0, strlen(cmd_buffer));
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t*)cmd_buffer, sizeof(cmd_buffer));
}



/*========== report i/o ===========*/

static void cell_voltage_preprocess(uint8_t cell_voltage_strip[], const uint32_t cell_voltage[IC_COUNT][18]) {
    int strip_index = 0;
    for(int ic = 0; ic < IC_COUNT; ic ++) {
        for(int cell = 0; cell < 18; cell ++) {
            if(!(1 << cell & CELL_MASK)) {
                continue;
            }
            cell_voltage_strip[strip_index ++] = cell_voltage[ic][cell] * 5e-5f; // 1uV/LSB to 20mV/LSB
        }
    }
}


/**
 * @param thermistor_strip  low_res output
 * @param thermistor        raw data input
 * @return max temperature in raw data format
 */
static float thermistor_preprocess(uint8_t thermistor_strip[], const float thermistor[IC_COUNT][32]) {
    int strip_index = 0;
    float max_temp = -300;
    for(int ic = 0; ic < IC_COUNT; ic ++) {
        for(int th_idx = 0; th_idx < 32; th_idx ++) {
            thermistor_strip[strip_index ++] = thermistor[ic][th_idx] + 32; // 1C/LSB + 32C
            max_temp = (thermistor[ic][th_idx] > max_temp)? thermistor[ic][th_idx] : max_temp;
        }
    }
    return max_temp;
}




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
) {
    // report type id
    enum report_type {
        FINISHED     = -1,
        CELL_VOLTAGE,
        THERMISTOR,
        HEARTBEAT,
        CELL_STATUS,
        STATE,
        REPORT_TYPE_COUNT // make sure this at enum tail
    };

    enum data_info {
        TOTAL_CELL          = IC_COUNT * 15,
        TOTAL_THERMISTOR    = IC_COUNT * 32,
    };

    static uint32_t tx_mail_box;
    static int reporting_type = FINISHED;
    const uint16_t can_base_id[REPORT_TYPE_COUNT] = {
        0x190, 0x390, 0x710, 0x290, 0x490
    };
    const uint32_t report_interval[REPORT_TYPE_COUNT] = { // ms, index map to reporting_id 0~N
        1000, 1000, 100, 200, 200
    };
    static uint32_t last_report[REPORT_TYPE_COUNT] = {0};
    static uint8_t cell_voltage_strip[TOTAL_CELL] = {0};
    static uint8_t thermistor_strip[TOTAL_THERMISTOR] = {0};
    static float max_temp = 0;

    uint8_t free_mailbox_count = HAL_CAN_GetTxMailboxesFreeLevel(hcan);
    while(free_mailbox_count) { // exit when mailbox full or report finish
        static int start_index;
        uint8_t data_frame[8] = {0};
        switch(reporting_type) {
        case FINISHED:
            // find id which reached interval
            for(int type = 0; type < REPORT_TYPE_COUNT; type ++) {
                if(HAL_GetTick() - last_report[type] >= report_interval[type]) {
                    last_report[type] = HAL_GetTick();
                    reporting_type = type;
                    start_index = 0;
                    // pre process
                    if(type == CELL_VOLTAGE) {
                        cell_voltage_preprocess(cell_voltage_strip, cell_voltage);
                    
                    }else if(type == THERMISTOR) {
                        max_temp = thermistor_preprocess(thermistor_strip, temperature);
                    }
                    break;
                }
            }
            // nothing to do
            if(reporting_type == FINISHED) {
                return;
            }
            continue;

        case CELL_VOLTAGE:
            can_tx_header->StdId = can_base_id[CELL_VOLTAGE];
            can_tx_header->DLC = 8;
            data_frame[0] = start_index;
            // exit when frame full or all cell reported
            for(int data_offset = 1; 
                data_offset < 8 && start_index < TOTAL_CELL; 
                data_offset ++, start_index ++
            ) {
                data_frame[data_offset] = cell_voltage_strip[start_index];
            }
            // this type finished
            if(start_index == TOTAL_CELL) {
                reporting_type = FINISHED;
            }
            break;

        case THERMISTOR:
            can_tx_header->StdId = can_base_id[THERMISTOR];
            can_tx_header->DLC = 8;
            data_frame[0] = start_index;
            // exit when frame full or all thermistor reported
            for(int data_offset = 1;
                data_offset < 8 && start_index < TOTAL_THERMISTOR; 
                data_offset ++, start_index ++
            ) {
                data_frame[data_offset] = thermistor_strip[start_index];
            }
            // this type finished
            if(start_index == TOTAL_THERMISTOR) {
                reporting_type = FINISHED;
            }
            break;

        case HEARTBEAT:
            can_tx_header->StdId = can_base_id[HEARTBEAT];
            can_tx_header->DLC = 1;
            data_frame[0] = 0x7F;
            // finish this type
            reporting_type = FINISHED;
            break;
        
        case CELL_STATUS:
            can_tx_header->StdId = can_base_id[CELL_STATUS];
            can_tx_header->DLC = 7;
            data_frame[0] = !(report_handle->info & 0x02);
            data_frame[1] = ((int16_t)max_temp) << 3;   // 0.125C/LSB, = raw*8 = raw << 3 
            data_frame[2] = ((int16_t)max_temp) >> 5;   // 0.125C/LSB, = raw*8 >> 8 = raw >> 5
            total_voltage *= 1.024e-3f; // uV to (1/1024)V/LSB
            data_frame[3] = total_voltage;
            data_frame[4] = total_voltage >> 8;
            data_frame[5] = total_voltage >> 16;
            data_frame[6] = total_voltage >> 24;
            // finish this type
            reporting_type = FINISHED;
            break;

        case STATE:
            can_tx_header->StdId = can_base_id[STATE];
            can_tx_header->DLC = 5;
            int16_t rp_current = current * 0.1;     // 1mA/LSB to 10mA/LSB
            int16_t rp_capacity = capacity * 2.77778e-7;   // 1uAs/LSB to 10mAh/LSB
            uint8_t soc = SOC_Transform(avg_cell_voltage * 1e-6f, 0) * 100.0f;
            data_frame[0] = soc;
            data_frame[1] = ((int16_t)rp_current);
            data_frame[2] = ((int16_t)rp_current) >> 8;
            data_frame[3] = ((int16_t)rp_capacity);
            data_frame[4] = ((int16_t)rp_capacity) >> 8;
            // finish this type
            reporting_type = FINISHED;
            break;
        }
        HAL_CAN_AddTxMessage(hcan, can_tx_header, data_frame, &tx_mail_box);
        // char log[64] = {0};
        // sprintf(log, "%lu | CAN TO <%03lX> (%lu|%u): %02X %02X %02X %02X %02X %02X %02X %02X\n", 
        //     HAL_GetTick(), can_tx_header->StdId, tx_mail_box, free_mailbox_count,
        //     data_frame[0], data_frame[1], data_frame[2], data_frame[3],
        //     data_frame[4], data_frame[5], data_frame[6], data_frame[7]
        // );
        // HAL_UART_Transmit(&huart1, (uint8_t*)log, strlen(log), 20);
        free_mailbox_count --;
    }
}




void uart_report_handle(
    uint32_t report_interval,
    report_handle_t *report_handle,
    const uint32_t cell_voltage[IC_COUNT][18], 
    const float temperature[IC_COUNT][32], 
    const adbms1818_stats_t stats[IC_COUNT]
) {
    static bool report_finished = true;
    static uint32_t last_report;
    
    // report config
    enum {
        BUFFER_SIZE         = 300,
        CELL_VOLTAGE_OFFSET = 5,
        TEMPERATURE_OFFSET  = 23,
        STATS_OFFSET        = 55,
        ERROR_INFO_OFFSET   = 62
    };

    if((HAL_GetTick() - last_report >= report_interval || !report_finished) && uart2_tx_dma_cplt) {
        static uint8_t ic_index;
        uint8_t data_frame[BUFFER_SIZE] = {0};

        // init report
        if(HAL_GetTick() - last_report >= report_interval) {
            last_report = HAL_GetTick();
            report_finished = false;
            ic_index = 0;
        }

        // header
        data_frame[0] = 0x4C;
        data_frame[1] = 0xA7;
        data_frame[2] = ic_index;
        // cell voltage
        for(int cell = 0; cell < 18; cell ++) {
            data_frame[CELL_VOLTAGE_OFFSET + cell] = cell_voltage[ic_index][cell] * 5e-5f;  // 1uV/LSB to 20mV/LSB
        }
        // temperature
        for(int th_idx = 0; th_idx < 32; th_idx ++) {
            data_frame[TEMPERATURE_OFFSET + th_idx] = (int)temperature[ic_index][th_idx] + 32;  // 1C/LSB + 32C
        }
        // stats
        uint16_t total_voltage  = stats[ic_index].SC * 1e-4f;      // 1uV/LSB to 10mV/LSB
        uint16_t analog_supply  = stats[ic_index].VA * 1e-3f;       // 1uV/LSB to 1mV/LSB
        uint16_t digital_supply = stats[ic_index].VD * 1e-3f;       // 1uV/LSB to 1mV/LSB
        data_frame[STATS_OFFSET + 0] = total_voltage;
        data_frame[STATS_OFFSET + 1] = total_voltage >> 8;
        data_frame[STATS_OFFSET + 2] = analog_supply;
        data_frame[STATS_OFFSET + 3] = analog_supply >> 8;
        data_frame[STATS_OFFSET + 4] = digital_supply;
        data_frame[STATS_OFFSET + 5] = digital_supply >> 8;
        data_frame[STATS_OFFSET + 6] = (int)stats[ic_index].ITMP + 32;
        // error info
        uint16_t error_info_cursor = 0;
        report_detail_t *rd = REPORT_DETAIL_FIRST(report_handle, ic_index);
        while(rd) {
            data_frame[ERROR_INFO_OFFSET + error_info_cursor ++] = rd->type;
            data_frame[ERROR_INFO_OFFSET + error_info_cursor ++] = rd->value;
            // go to next rd
            rd = rd->next;
        }
        // data length
        uint16_t data_length = ERROR_INFO_OFFSET + error_info_cursor - CELL_VOLTAGE_OFFSET;
        data_frame[3] = data_length;
        data_frame[4] = data_length >> 8;
        // uart send
        // HAL_UART_Transmit(&huart1, data_frame, CELL_VOLTAGE_OFFSET + data_length, 100);
        if(uart_debug_mode == UART_INFO_MODE) {
            uart2_tx_dma_cplt = false;
            HAL_UART_Transmit_DMA(&huart1, data_frame, CELL_VOLTAGE_OFFSET + data_length);
            // HAL_UART_Transmit(&huart1, data_frame, CELL_VOLTAGE_OFFSET + data_length, 100);
        }
        // next ic
        ic_index ++;
        if(ic_index >= IC_COUNT) {
            report_finished = true;
        }
    }
}