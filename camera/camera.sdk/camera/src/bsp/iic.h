/*
 * iic.h
 *
 *  Created on: Jan 15, 2019
 *      Author: Ilija
 */

#ifndef SRC_BSP_IIC_H_
#define SRC_BSP_IIC_H_

#include "xiicps.h"

#define IIC_ADDRESS            0x21


XIicPs i2c;

int init_i2c();
int I2C_read(u8 register_offset, u8 *read_value);
int I2C_write(u8 register_offset, u8 write_value);


#endif /* SRC_BSP_IIC_H_ */
