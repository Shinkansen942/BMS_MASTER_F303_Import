#include "cat24c.h"
#include <string.h>

#define HAL_CHECK(x)    do{ if((x) != HAL_OK) {return CAT24C_ERR;} }while(0);
#define CAT24C_CHECK(x) do{ if((x) != CAT24C_OK) {return (x);} }while(0);

#define REAL_ADDR(ext_addr) (0xA0 | (((ext_addr) & 0x07) << 1)) 


static cat24c_status_type_t _cat24c_write_bytes(const cat24c_handle_t *cat24c, uint16_t addr, uint8_t* data, uint16_t size) {
    uint16_t buffer_size = size + 2;
    uint8_t buf[buffer_size];
    buf[0] = addr >> 8;
    buf[1] = addr;
    memcpy(&buf[2], data, size);
    HAL_CHECK(HAL_I2C_Master_Transmit(cat24c->hi2c, REAL_ADDR(cat24c->external_addr), buf, buffer_size, 10));
    HAL_Delay(5);
    return CAT24C_OK;
}



cat24c_status_type_t cat24c_read_bytes(const cat24c_handle_t *cat24c, uint16_t addr, void *data, uint16_t size) {
    uint8_t buf[2];
    buf[0] = addr >> 8;
    buf[1] = addr;
    HAL_CHECK(HAL_I2C_Master_Transmit(cat24c->hi2c, REAL_ADDR(cat24c->external_addr), buf, 2, 10));
    HAL_CHECK(HAL_I2C_Master_Receive(cat24c->hi2c, REAL_ADDR(cat24c->external_addr), data, size, 100));
    return CAT24C_OK;
}



cat24c_status_type_t cat24c_write_bytes(const cat24c_handle_t *cat24c, uint16_t addr, const void *data, uint16_t size, bool verify) {
    uint16_t write_size_left = size;
    // write first page
    uint16_t first_page_left = cat24c->page_size - (addr % cat24c->page_size);
    CAT24C_CHECK(_cat24c_write_bytes(cat24c, addr, (uint8_t*)data, first_page_left));
    write_size_left -= first_page_left; 
    // write other page
    uint8_t* data_offseted = (uint8_t*)data + first_page_left;
    uint16_t addr_offseted = addr + first_page_left;
    while(write_size_left) {
        // equal to min(page_size, write_size_left)
        uint16_t size_to_write = write_size_left > cat24c->page_size ? cat24c->page_size : write_size_left;
        CAT24C_CHECK(_cat24c_write_bytes(cat24c, addr_offseted, data_offseted, size_to_write));
        write_size_left -= size_to_write;
        data_offseted += size_to_write;
        addr_offseted += size_to_write;
    }
    // verify
    if(verify) {
        uint8_t vfy_buf[size];
        CAT24C_CHECK(cat24c_read_bytes(cat24c, addr, vfy_buf, size))
        if(memcmp(vfy_buf, data, size) != 0) {
            return CAT24C_VERIFY_FAILED;
        }
    }
    return CAT24C_OK;
}