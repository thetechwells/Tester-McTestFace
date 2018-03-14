#ifndef I2C_H
#define I2C_H

#include <QObject>
#include <QDebug>
#include "wiringPi.h"
#include "wiringPiI2C.h"

class i2c : public QObject
{
    Q_OBJECT
public:
    i2c();

    unsigned int stepperSlaveAddress;
    unsigned int cbrSlaveAddress;

    void writeData_I2C(unsigned int, uint8_t, uint8_t, uint8_t);
    uint16_t readData_I2C(unsigned int, uint8_t);

private:

signals:

public slots:

};

#endif // I2C_H
