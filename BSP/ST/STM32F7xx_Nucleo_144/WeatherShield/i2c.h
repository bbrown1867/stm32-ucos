/**
 * @file   i2c.h
 * @author Ben Brown <ben@beninter.net>
 * @brief  I2C shims for the MS8607 driver.
 */

#ifndef I2C_H
#define I2C_H

#include <stdint.h>

enum status_code
{
    STATUS_OK           = 0x00,
    STATUS_ERR_OVERFLOW	= 0x01,
    STATUS_ERR_TIMEOUT  = 0x02,
};

struct i2c_master_packet
{
    uint16_t address;
    uint16_t data_length;
    uint8_t  *data;
};

void             delay_ms                            (uint32_t duration_ms);
void             i2c_master_init                     (void);
enum status_code i2c_master_read_packet_wait         (struct i2c_master_packet *const packet);
enum status_code i2c_master_write_packet_wait        (struct i2c_master_packet *const packet);
enum status_code i2c_master_write_packet_wait_no_stop(struct i2c_master_packet *const packet);

#endif /* I2C_H */
