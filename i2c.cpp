#include "i2c.h"

i2c::i2c()
{
    stepperSlaveAddress = wiringPiI2CSetup(0x2D);
    //cbrSlaveAddress = wiringPiI2CSetup(0xC5);
}

/**
 * @brief i2c::writeData_I2C
 * @param commandCode
 * @param msgLSB
 * @param msgMSB
 */
void i2c::writeData_I2C(unsigned int slaveAddress, uint8_t commandCode, uint8_t msgLSB, uint8_t msgMSB)
{
    uint16_t data = 0x00;
    data = msgMSB;
    data <<= 8;
    data = data + msgLSB;
    wiringPiI2CWriteReg16(slaveAddress, commandCode, data);
}

/**
 * @brief i2c::readData_I2C
 * @param commandCode
 * @return
 */
uint16_t i2c::readData_I2C(unsigned int slaveAddress, uint8_t commandCode)
{
    uint16_t data = 0x00;
    data = wiringPiI2CReadReg16(slaveAddress, commandCode);
    return data;
}
