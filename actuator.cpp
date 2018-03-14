#include "actuator.h"

static int direction = 1;

actuator::actuator(QObject *parent) : QObject(parent)
{
    actuatorTimer = new timeManager();
}

actuator::~actuator()
{

}

/**
 * @brief actuator::messageFrame
 * @param message
 */
void actuator::messageFrame(QByteArray chMsg)
{
    QByteArray msgOut = ":";
    bool ok;

    if (chMsg.mid(2, 6) == "060D00" || lastMsg.msg.mid(2, 6) == "060D00") // 05040B = HOME command, 060D00 = Home start command lmao
    {
        gateCounter = 0;
        lastMsg.lastMoveMsg = "00000000";
    }
    else if (chMsg.mid(2, 6) == "109900") // 109900 = MOVE command
    {
        //if we're moving more forward than our last position
        if (lastMsg.lastMoveMsg.toInt(&ok, 16) < chMsg.mid(14, 8).toInt(&ok, 16))
            direction = FORWARD;
        else
            direction = BACKWARD;
    }
    else if (chMsg.mid(2, 6) == "050416") //050416 = JOG+
    {
        direction = FORWARD;
    }
    else if (chMsg.mid(2, 6) == "050417") //050417 = JOG-
    {
        direction = BACKWARD;
    }

    lastMsg.msg = chMsg;

    if (lastMsg.msg.mid(2, 6) == "109900")
    {
        lastMsg.lastMoveMsg = lastMsg.msg.mid(14, 8);
    }

    msgOut.append(chMsg);
    msgOut.append(LRC(chMsg));
    msgOut.append("\r\n");
    emit sigMsgOut(msgOut);
}

/**
 * @brief actuator::LRC Longitudinal redundancy check.
 * This format uses two's complement to form ASCII hex values.
 * @param auchMsg Original input message and returns the appropriate LRC.
 * @return QbyteArray ASCII hex
 */
QByteArray actuator::LRC(QByteArray auchMsg)
{
    qint8 uchLRC = 0;
    quint8 dec1 = 0;
    unsigned int tick = 0;
    unsigned int place = 0;
    quint8 decMsg[256];
    QByteArray calculated;

    for (int i = 0; i < auchMsg.length(); i++)
    {
        quint8 dec = auchMsg.at(i);

        if (dec >= 48 && dec <= 57)
        {
            dec -= 48;
        }
        else if (dec >= 65 && dec <= 70)
        {
            dec -= 55;
        }

        if (tick == 0)
        {
            dec1 = dec * 16;
            tick += 1;
        }
        else
        {
            decMsg[place] = dec1 + dec;
            place += 1;
            tick = 0;
        }

    }

    for (unsigned int i = 0; i < place; i++)
    {
        uchLRC += decMsg[i];
    }

    uchLRC = ~uchLRC; // perform a 2s complement on the nuumber.
    uchLRC += 1; // add 1 to number to finish 2s complement.

    calculated.append(uchLRC);
    calculated = calculated.toHex().toUpper();
    return calculated;
}

/**
 * @brief actuator::sltMsgIn Message response parsing.
 * Signal is recieved and message goes through parsing to determin message destination.
 * @param msgResponse The response message from the IAI controller.
 */
void actuator::sltMsgIn(QByteArray msgResponse)
{
    quint8 dec1 = 0;
    QByteArray reLRC;
    QByteArray msgLRC;
    unsigned int tick = 0;
    funCode = 0;
    numOfBytes = 0;
    dataReturn = 0;

    if (msgResponse.length() > 8)
    {
        msgResponse.remove(0, 1); // cut the header and slave address off the message.
        msgResponse.chop(2); // Chop control chars of end of message.
        msgLRC = msgResponse.remove((msgResponse.length() - 2), 2); // Get the sent LRC response.
        reLRC = LRC(msgResponse); // Calculate the LRC ourselves.

        qDebug() << msgResponse << msgLRC << reLRC;

        if (reLRC == msgLRC)
        {

            for (int i = 0; i < 2; i++)
            {
                quint8 dec = msgResponse.at(i);

                if (dec >= 48 && dec <= 57)
                {
                    dec -= 48;
                }
                else if (dec >= 65 && dec <= 70)
                {
                    dec -= 55;
                }

                if (tick == 0)
                {
                    dec1 = dec * 16;
                    tick += 1;
                }
                else
                {
                    funCode = dec1 + dec;
                    tick = 0;
                    msgResponse.remove(0, 2);
                }
            }

            dec1 = 0;

            if (funCode == 3)
            {
                for (int i = 0; i < 2; i++)
                {
                    quint8 dec = msgResponse.at(i);

                    if (dec >= 48 && dec <= 57)
                    {
                        dec -= 48;
                    }
                    else if (dec >= 65 && dec <= 70)
                    {
                        dec -= 55;
                    }

                    if (tick == 0)
                    {
                        dec1 = dec * 16;
                        tick += 1;
                    }
                    else
                    {
                        numOfBytes = dec1 + dec;
                        tick = 0;
                        msgResponse.remove(0, 2);
                    }
                }
                dataReturn = msgResponse; // return only the meat and potatoes of the message
                qDebug() << dataReturn;
            }
            else
            {
                dataReturn = "";
            }
        }
    }
}

/**
 * @brief actuator::HOME Homing program for the IAI actuator.
 *	The homing program resets the actuators internal positioning to its true zero.
 *	Servo must be set to ON before function call. Will fail and possiably cause alarm if not.
 *	The program sets the homing bit in reg 0D00 to 1 and activates on rising edge. (see IAI manual ME0162-9A for more device information)
 *	The actuator address 9007 is queried for the is homing bit to return to normal.
 *	The program ends with setting the homeing bit back to normal value. (normally off)
 */
void actuator::HOME()
{
    bool yes = true;
    dataReturn = "";

    /** Set the Home return bit to '1' start the home return. */
    messageFrame("01060D001010");

    while (yes)							/** While moving do nothing. This is a program safety method. */
    {
        readReg9007();
        actuatorTimer->waitMs(100);
        if (dataReturn.length() > 2)
        {
            if (dataReturn.at(1) != 'B')
            {
                yes = false;
            }
        }
    }
    /** Set the Home return bit back to '0' to reset the rising edge detector method. */
    messageFrame("01060D001000");
    actuatorTimer->waitMs(5);
}

/**
 * @brief actuator::MODBUS
 *  Turns off the PIO functionaliy.
 *  Switches to the modbus serial and disables the actuator PIO.
 *  Needs only to be set once. Ever.
 *  return void
 */
void actuator::MODBUS()
{
    messageFrame("01050427FF00");
}

/**
 * @brief actuator::alarmClr Clears any triggered alarms.
 *	Resets the internal alarms.
 *	If the alarms do not clear then the issue that has triggered the alarm has not been remidied.
 */
void actuator::alarmClr()
{
    messageFrame("01050407FF00");
    messageFrame("010504070000");
}

/**
 * @brief actuator::servoON Turns the servo on.
 *	Sets the servo bit to 1 in address 0D00.
 *	return void
 */
void actuator::servoON()
{
    messageFrame("01060D001000");
}

/**
 * @brief actuator::servoOFF Turns the servo off.
 *	Sets the servo bet to 0.
 *	return void
 */
void actuator::servoOFF()
{
    messageFrame("010504030000");
}

/**
 * @brief actuator::posNOW Gets the current actuator head position.
 *	Queries the actuator reg 9000.
 *	Parses the return message location value.
 * @return location
 */
float actuator::posNOW()
{
    messageFrame("010390000002");
    dataReturn = "";
    actuatorTimer->waitMs(40);
    bool ok;
    actuatorTimer->startTimeWhile();
    while ((dataReturn == "") && (actuatorTimer->endTimeWhile() != 40000))
    {

    }
    int num = dataReturn.toInt(&ok, 16);
    return (num * 0.01f);
}

/**
 * @brief actuator::moveJogPos Starts the Jog+ movement.
 *	Sets the Jog possitive bit in reg 0416.
 *	return void
 */
void actuator::moveJogPos()
{
    messageFrame("01050416FF00");
}

/**
 * @brief actuator::moveJogPosStop Stops the Jog+ movement.
 *	Clears the Jog possitive bit in reg 0416.
 * 	return void
 */
void actuator::moveJogPosStop()
{
    messageFrame("010504160000");
}

/**
 * @brief actuator::moveJogNeg Starts the Jog- movement.
 *	Sets the Jog negitive bit in reg 0417.
 *	return void
 */
void actuator::moveJogNeg()
{
    messageFrame("01050417FF00");
}

/**
 * @brief actuator::moveJogNegStop Stops the Jog- movement.
 *	Clears the Jog negitive bit in reg 0417.
 * 	return void
 */
void actuator::moveJogNegStop()
{
    messageFrame("010504170000");
}

/**
 * @brief actuator::moveInc Starts an incrimental movement for 50 moves.
 * @todo reformat the function to accept parameters for movement.
 */
void actuator::moveInc()
{
    /*
    int count = 0;
    running = true;

    while (count < 50)
    {
        if (running)
        {
            messageFrame("01109900000912000000640000000A00002710001E00000008");
            count++;
        }
        else
        {
            break;
        }
    }
    */
}

/**
 * @brief actuator::moveStop Stops all movement.
 *	Sets the movement pause bit in reg 040A.
 *	All movement is paused until the bit is cleared.
 *	Last set movement is completed or started over.
 */
void actuator::moveStop()
{
    messageFrame("0105040AFF00");
}

/**
 * @brief actuator::isMoving Queries the controller movement bit.
 *	seaks whether the actuator is moving or not.
 *	If moving stay in method.
 *	Exits when bit is cleared.
 */
void actuator::isMoving()
{
    actuatorTimer->startTimeWhile();

    while (actuatorTimer->endTimeWhile() < 10000)
    {
        readReg9007();
        actuatorTimer->waitMs(60);
        if (dataReturn.length() > 2)
        {
            if (dataReturn.at(2) != 'F')
            {
                break;
            }
        }
    }
}

/**
 * @brief actuator::readReg9007 Reads the memory address 9007.
 *	Quries the reg 9007.
 */
void actuator::readReg9007()
{
    messageFrame("010390070001");
}

/**
 * @brief actuator::moveTo move actuator head to variable point.
 *  Moves the actuator head to the point that is set via serial communication.
 * @param target
 * @param posBand
 * @param speed
 * @param acceleration
 */
void actuator::moveTo(float target, float posBand, float speed, float acceleration)
{
    QByteArray chMsg;
    //qDebug() << target << posBand << speed << acceleration;
    chMsg = "01";   // Slave Address
    chMsg.append(IAI_CODE_POS_WRITE); // Function Code
    chMsg.append("9900");   // Start Address
    chMsg.append("0007");   // Number of registers
    chMsg.append("0E"); // Number of bytes
    chMsg.append(targetPosToHex(target));    // Target position
    chMsg.append(posBandToHex(posBand));   // Position Precision Band
    chMsg.append(speedToHex(speed));  // Movement Speed
    chMsg.append(accelToHex(acceleration));  // Acceleration and deceleration... gravitation acceleration.

    //qDebug() << chMsg << "message";
    messageFrame(chMsg);
    isMoving();
    //gateCounter = 0;
    //direction = FORWARD;
}

/**
 * @brief actuator::pushToMsg
 * @param slaveAddress
 * @param target
 * @param band
 * @param speed
 * @param accel
 * @param pushStrength
 * @return
 */
QByteArray actuator::pushToMsg(QString slaveAddress, float target, float band, float speed, float accel, float pushStrength)
{
    QByteArray query;
    query.append(slaveAddress);
    query.append(IAI_CODE_POS_WRITE);
    query.append("9900000912"); // Message information... Needs to be cleaner... I don't like this yet... Be better. Get Gooder.
    query.append(targetPosToHex(target)); // default = 10
    query.append(posBandToHex(band)); // default = 140
    query.append(speedToHex(speed)); // default = 30
    query.append(accelToHex(accel)); // default = 0.3
    query.append(pushToHex(pushStrength)); // default = 0.20
    query.append(controlToHex(true,true,false));
    return query;
}

/**
 * @brief actuator::moveToMsg builds message for movement.
 * @param target
 * @param posBand
 * @param speed
 * @param acceleration
 * @return
 */
QByteArray actuator::moveToMsg(float target, float posBand, float speed, float acceleration)
{
    QByteArray query;
    query = "01";   // Slave Address
    query.append(IAI_CODE_POS_WRITE); // Function Code
    query.append("9900");   // Start Address
    query.append("0007");   // Number of registers
    query.append("0E"); // Number of bytes
    query.append(targetPosToHex(target));    // Target position
    query.append(posBandToHex(posBand));   // Position Precision Band
    query.append(speedToHex(speed));  // Movement Speed
    query.append(accelToHex(acceleration));  // Acceleration and deceleration... gravitation acceleration.
    return query;
}

/**
 * @brief actuator::pushToHex
 * @param percent range from 0.7 - 0.2
 * @return
 */
QByteArray actuator::pushToHex(float percent)
{
    QByteArray percentHex;

    // Push values are 20% - 70%
    if (percent <= 0.70 && percent >= 0.20)
    {
        // Push data is 255 = 100%
        // Push value 255 * percent = dec
        // dec to hex and you have your value.
        quint8 percentDec = 255;
        percentDec = percentDec * percent;
        percentHex.append(QString::number(percentDec,16));
        //percentHex = percentHex.toHex();
        percentHex = percentHex.toUpper();

    }
    else
    {
        // Return all zeros if not a valid value.
        percentHex = "0000";
    }

    if (percentHex.length() != 4)   // Makes the return value have 4 ascii characters. This is needed for bit consistancy in the message.
    {
        if (percentHex.length() == 1)
        {
            percentHex.prepend("000");
        }
        if (percentHex.length() == 2)
        {
            percentHex.prepend("00");
        }
        if (percentHex.length() == 3)
        {
            percentHex.prepend("0");
        }
    }

    //qDebug() << percentHex;

    return percentHex;
}

/**
 * @brief actuator::accelToHex
 * @param gravity
 * @return
 */
QByteArray actuator::accelToHex(float gravity)
{
    QByteArray data_array;  // Data speed select.

    if (gravity > 0 && gravity <= 1)
    {
        quint16 gravityDec = gravity * 100;
        data_array.append(QString::number(gravityDec, 16));
        //data_array = data_array.toHex();
        data_array = data_array.toUpper();
    }
    else
    {
        data_array = "0000";
    }

    if (data_array.length() != 4)   // Makes the return value have 4 ascii characters. This is needed for bit consistancy in the message.
    {
        if (data_array.length() == 1)
        {
            data_array.prepend("000");
        }
        if (data_array.length() == 2)
        {
            data_array.prepend("00");
        }
        if (data_array.length() == 3)
        {
            data_array.prepend("0");
        }
    }

    return data_array;
}

/**
 * @brief actuator::targetPosToHex
 * @param target
 * @return
 */
QByteArray actuator::targetPosToHex(float target)
{
    QByteArray data_array = 0;

    if (target <= 150 && target >= 0)
    {
        quint16 targetDec = (target * 100);
        data_array.append(QString::number(targetDec,16));
        //data_array = data_array.toHex();
        data_array = data_array.toUpper();
    }
    else
    {
        data_array = "0000";
    }

    if (data_array.length() != 4)   // Makes the return value have 8 ascii characters. This is needed for bit consistancy in the message.
    {
        if (data_array.length() == 1)
        {
            data_array.prepend("0000000");
        }
        else if (data_array.length() == 2)
        {
            data_array.prepend("000000");
        }
        else if (data_array.length() == 3)
        {
            data_array.prepend("00000");
        }
    }
    else
    {
        data_array.prepend("0000");
    }

    return data_array;
}

/**
 * @brief actuator::speedToHex
 * @param speed
 * @return
 */
QByteArray actuator::speedToHex(float speed)
{
    QByteArray data_array;

    if (speed <= 250 && speed >= 0)
    {
        quint16 speedDec = speed * 100;
        data_array.append(QString::number(speedDec,16));
        //data_array = data_array.toHex();
        data_array = data_array.toUpper();
    }
    else
    {
        data_array = "0000";
    }

    if (data_array.length() != 4) // Makes the return value have 8 ascii characters. This is needed for bit consistancy in the message.
    {
        if (data_array.length() == 1)
        {
            data_array.prepend("0000000");
        }
        else if (data_array.length() == 2)
        {
            data_array.prepend("000000");
        }
        else if (data_array.length() == 3)
        {
            data_array.prepend("00000");
        }
    }
    else
    {
        data_array.prepend("0000");
    }

    return data_array;
}

/**
 * @brief actuator::posBandToHex
 * @param band
 * @return
 */
QByteArray actuator::posBandToHex(float band)
{
    QByteArray data_array;


    if (band >= 0.1f && band <= 150.0f)
    {
        quint16 temp = (quint16)(band * 100.0);
        data_array.append(QString::number(temp, 16));
        //data_array = data_array.toHex();
        data_array = data_array.toUpper();
        //qDebug() << "Band" << band << temp;
    }
    else if (band < 0.1f)
    {
        quint16 temp = 1;
        data_array.append(QString::number(temp, 16));
        //data_array = data_array.toHex();
        data_array = data_array.toUpper();
        //qDebug() << "Band" << band << temp;
    }
    else
    {
        data_array = "0000";
    }

    if (data_array.length() != 4)   // Makes the return value have 8 ascii characters. This is needed for bit consistancy in the message.
    {
        if (data_array.length() == 1)
        {
            data_array.prepend("0000000");
        }
        else if (data_array.length() == 2)
        {
            data_array.prepend("000000");
        }
        else if (data_array.length() == 3)
        {
            data_array.prepend("00000");
        }
    }
    else
    {
        data_array.prepend("0000");
    }

    return data_array;
}

/**
 * @brief actuator::controlToHex
 * @param push  True is Push operation.
 *              False is Normal operation (default).
 * @param dir   True is fhe direction of push-motion operation after completion of approach is
 *              defined as the reverse direction.
 *              False is the direction of push-motion operation after completion of approach is
 *              defined as the forward direction (default).
 * @param inc   True is incremental operation (pitch feed).
 *              False is normal operation (default).
 * @return QByteArray of Control Status Registers commands.
 */
QByteArray actuator::controlToHex(bool push, bool dir, bool inc)
{
    QByteArray data_array;
    quint16 controlReg = 0;

    if (push == true)
    {
        if (dir == true)
        {
            controlReg = 0b110;
        }
        else
        {
            controlReg = 0b010;
        }
    }
    if ((push == false) && (inc == true))
    {
        controlReg = (controlReg | 0b1000);
    }

    data_array.append(QString::number(controlReg,16).toUpper());

    if (data_array.length() != 4)   // Makes the return value have 4 ascii characters. This is needed for bit consistancy in the message.
    {
        if (data_array.length() == 1)
        {
            data_array.prepend("000");
        }
        if (data_array.length() == 2)
        {
            data_array.prepend("00");
        }
        if (data_array.length() == 3)
        {
            data_array.prepend("0");
        }
    }

    //qDebug() << data_array;
    return data_array;
}

void actuator::sltServoOnOff(int On)
{
    if(1 == On)
    {
        servoON();
    }
    else
    {
        servoOFF();
    }
}

void actuator::sltHome()
{
    HOME();
}
