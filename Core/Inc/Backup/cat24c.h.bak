#ifndef __CAT24C_H__
#define __CAT24C_H__

#include "i2c.h"
#include "stdbool.h"

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint16_t page_size;
    uint8_t external_addr; // bits: A2 A1 A0
}cat24c_handle_t;


typedef enum {
    CAT24C_ERR,
    CAT24C_VERIFY_FAILED,
    CAT24C_OK
}cat24c_status_type_t;


cat24c_status_type_t cat24c_write_bytes(const cat24c_handle_t *cat24c, uint16_t addr, const void *data, uint16_t size, bool verify);

// cat24c_status_type_t cat24c_write_page(const cat24c_handle_t *cat24c, uint16_t page, const void *data, uint16_t size, bool verify);

cat24c_status_type_t cat24c_read_bytes(const cat24c_handle_t *cat24c, uint16_t addr, void *data, uint16_t size);

// cat24c_status_type_t cat24c_read_page(const cat24c_handle_t *cat24c, uint16_t page, void *data, uint16_t size);


#endif