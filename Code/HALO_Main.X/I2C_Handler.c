/*
 * File:   I2C_Handler.c
 * Author: Hunter Hykes
 *
 * Created on November 5, 2020, 7:41 PM
 */

#include "xc.h"
#include "mcc_generated_files/mcc.h"

#define SX1505_RETRY_MAX       100  // define the retry count
#define SX1505_ADDRESS         0x20 // slave device address
#define SX1505_DEVICE_TIMEOUT  50   // define slave timeout 

//#define BMA250_RETRY_MAX       100  // define the retry count
//#define BMA250_ADDRESS         0x18 // slave device address
//#define BMA250_DEVICE_TIMEOUT  50   // define slave timeout 

#define H3LIS200DL_RETRY_MAX       100  // define the retry count
#define H3LIS200DL_ADDRESS         0x18 // slave device address
#define H3LIS200DL_DEVICE_TIMEOUT  50   // define slave timeout 

#define VL53L0X_RETRY_MAX       100  // define the retry count
#define VL53L0X_ADDRESS         0x29 // slave device address
#define VL53L0X_DEVICE_TIMEOUT  50   // define slave timeout 

uint8_t SX1505_Read(
        uint16_t address,
        uint8_t *pData,
        uint16_t nCount) {
    I2C1_MESSAGE_STATUS status;
    uint8_t writeBuffer[3];
    uint16_t retryTimeOut, slaveTimeOut;
    uint16_t counter;
    uint8_t *pD;

    pD = pData;

    // can get rid of for loop and call I2CMasterRead with for nCount Bytes
    for (counter = 0; counter < nCount; counter++) {

        // build the write buffer first
        // starting address of the EEPROM memory
        writeBuffer[0] = (address >> 8); // high address
        writeBuffer[1] = (uint8_t) (address); // low low address

        // Now it is possible that the slave device will be slow.
        // As a work around on these slaves, the application can
        // retry sending the transaction
        retryTimeOut = 0;
        slaveTimeOut = 0;

        // write to slave to move memory pointer
        while (status != I2C1_MESSAGE_FAIL) {
            // write one byte to EEPROM (2 is the count of bytes to write)
            I2C1_MasterWrite(writeBuffer,
                    2,
                    SX1505_ADDRESS,
                    &status);

            // wait for the message to be sent or status has changed.
            while (status == I2C1_MESSAGE_PENDING) {
                // add some delay here

                // timeout checking
                // check for max retry and skip this byte
                if (slaveTimeOut == SX1505_DEVICE_TIMEOUT)
                    return (0);
                else
                    slaveTimeOut++;
            }

            if (status == I2C1_MESSAGE_COMPLETE)
                break;

            // if status is  I2C1_MESSAGE_ADDRESS_NO_ACK,
            //               or I2C1_DATA_NO_ACK,
            // The device may be busy and needs more time for the last
            // write so we can retry writing the data, this is why we
            // use a while loop here

            // check for max retry and skip this byte
            if (retryTimeOut == SX1505_RETRY_MAX)
                break;
            else
                retryTimeOut++;
        }

        if (status == I2C1_MESSAGE_COMPLETE) {

            // this portion will read the byte from the memory location.
            retryTimeOut = 0;
            slaveTimeOut = 0;

            while (status != I2C1_MESSAGE_FAIL) {
                // write one byte to EEPROM (2 is the count of bytes to write)
                I2C1_MasterRead(pD,
                        1,
                        SX1505_ADDRESS,
                        &status);

                // wait for the message to be sent or status has changed.
                while (status == I2C1_MESSAGE_PENDING) {
                    // add some delay here

                    // timeout checking
                    // check for max retry and skip this byte
                    if (slaveTimeOut == SX1505_DEVICE_TIMEOUT)
                        return (0);
                    else
                        slaveTimeOut++;
                }

                if (status == I2C1_MESSAGE_COMPLETE)
                    break;

                // if status is  I2C1_MESSAGE_ADDRESS_NO_ACK,
                //               or I2C1_DATA_NO_ACK,
                // The device may be busy and needs more time for the last
                // write so we can retry writing the data, this is why we
                // use a while loop here

                // check for max retry and skip this byte
                if (retryTimeOut == SX1505_RETRY_MAX)
                    break;
                else
                    retryTimeOut++;
            }
        }

        // exit if the last transaction failed
        if (status == I2C1_MESSAGE_FAIL) {
            return (0);
            break;
        }

        pD++;
        address++;

    }
    return (1);

}

uint8_t H3LIS200DL_Read(
        uint16_t address,
        uint8_t *pData,
        uint16_t nCount) {
    I2C1_MESSAGE_STATUS status;
    uint8_t writeBuffer[3];
    uint16_t retryTimeOut, slaveTimeOut;
    uint16_t counter;
    uint8_t *pD;

    pD = pData;

    // can get rid of for loop and call I2CMasterRead with for nCount Bytes
    for (counter = 0; counter < nCount; counter++) {

        // build the write buffer first
        // starting address of the EEPROM memory
        writeBuffer[0] = (address >> 8); // high address
        writeBuffer[1] = (uint8_t) (address); // low low address

        // Now it is possible that the slave device will be slow.
        // As a work around on these slaves, the application can
        // retry sending the transaction
        retryTimeOut = 0;
        slaveTimeOut = 0;

        // write to slave to move memory pointer
        while (status != I2C1_MESSAGE_FAIL) {
            // write one byte to EEPROM (2 is the count of bytes to write)
            I2C1_MasterWrite(writeBuffer,
                    2,
                    H3LIS200DL_ADDRESS,
                    &status);

            // wait for the message to be sent or status has changed.
            while (status == I2C1_MESSAGE_PENDING) {
                // add some delay here

                // timeout checking
                // check for max retry and skip this byte
                if (slaveTimeOut == H3LIS200DL_DEVICE_TIMEOUT)
                    return (0);
                else
                    slaveTimeOut++;
            }

            if (status == I2C1_MESSAGE_COMPLETE)
                break;

            // if status is  I2C1_MESSAGE_ADDRESS_NO_ACK,
            //               or I2C1_DATA_NO_ACK,
            // The device may be busy and needs more time for the last
            // write so we can retry writing the data, this is why we
            // use a while loop here

            // check for max retry and skip this byte
            if (retryTimeOut == H3LIS200DL_RETRY_MAX)
                break;
            else
                retryTimeOut++;
        }

        if (status == I2C1_MESSAGE_COMPLETE) {

            // this portion will read the byte from the memory location.
            retryTimeOut = 0;
            slaveTimeOut = 0;

            while (status != I2C1_MESSAGE_FAIL) {
                // write one byte to EEPROM (2 is the count of bytes to write)
                I2C1_MasterRead(pD,
                        1,
                        H3LIS200DL_ADDRESS,
                        &status);

                // wait for the message to be sent or status has changed.
                while (status == I2C1_MESSAGE_PENDING) {
                    // add some delay here

                    // timeout checking
                    // check for max retry and skip this byte
                    if (slaveTimeOut == H3LIS200DL_DEVICE_TIMEOUT)
                        return (0);
                    else
                        slaveTimeOut++;
                }

                if (status == I2C1_MESSAGE_COMPLETE)
                    break;

                // if status is  I2C1_MESSAGE_ADDRESS_NO_ACK,
                //               or I2C1_DATA_NO_ACK,
                // The device may be busy and needs more time for the last
                // write so we can retry writing the data, this is why we
                // use a while loop here

                // check for max retry and skip this byte
                if (retryTimeOut == H3LIS200DL_RETRY_MAX)
                    break;
                else
                    retryTimeOut++;
            }
        }

        // exit if the last transaction failed
        if (status == I2C1_MESSAGE_FAIL) {
            return (0);
            break;
        }

        pD++;
        address++;

    }
    return (1);

}

uint8_t VL53L0X_Read(
        uint16_t address,
        uint8_t *pData,
        uint16_t nCount) {
    I2C1_MESSAGE_STATUS status;
    uint8_t writeBuffer[3];
    uint16_t retryTimeOut, slaveTimeOut;
    uint16_t counter;
    uint8_t *pD;

    pD = pData;

    // can get rid of for loop and call I2CMasterRead with for nCount Bytes
    for (counter = 0; counter < nCount; counter++) {

        // build the write buffer first
        // starting address of the EEPROM memory
        writeBuffer[0] = (address >> 8); // high address
        writeBuffer[1] = (uint8_t) (address); // low low address

        // Now it is possible that the slave device will be slow.
        // As a work around on these slaves, the application can
        // retry sending the transaction
        retryTimeOut = 0;
        slaveTimeOut = 0;

        // write to slave to move memory pointer
        while (status != I2C1_MESSAGE_FAIL) {
            // write one byte to EEPROM (2 is the count of bytes to write)
            I2C1_MasterWrite(writeBuffer,
                    2,
                    VL53L0X_ADDRESS,
                    &status);

            // wait for the message to be sent or status has changed.
            while (status == I2C1_MESSAGE_PENDING) {
                // add some delay here

                // timeout checking
                // check for max retry and skip this byte
                if (slaveTimeOut == VL53L0X_DEVICE_TIMEOUT)
                    return (0);
                else
                    slaveTimeOut++;
            }

            if (status == I2C1_MESSAGE_COMPLETE)
                break;

            // if status is  I2C1_MESSAGE_ADDRESS_NO_ACK,
            //               or I2C1_DATA_NO_ACK,
            // The device may be busy and needs more time for the last
            // write so we can retry writing the data, this is why we
            // use a while loop here

            // check for max retry and skip this byte
            if (retryTimeOut == VL53L0X_RETRY_MAX)
                break;
            else
                retryTimeOut++;
        }

        if (status == I2C1_MESSAGE_COMPLETE) {

            // this portion will read the byte from the memory location.
            retryTimeOut = 0;
            slaveTimeOut = 0;

            while (status != I2C1_MESSAGE_FAIL) {
                // write one byte to EEPROM (2 is the count of bytes to write)
                I2C1_MasterRead(pD,
                        1,
                        VL53L0X_ADDRESS,
                        &status);

                // wait for the message to be sent or status has changed.
                while (status == I2C1_MESSAGE_PENDING) {
                    // add some delay here

                    // timeout checking
                    // check for max retry and skip this byte
                    if (slaveTimeOut == VL53L0X_DEVICE_TIMEOUT)
                        return (0);
                    else
                        slaveTimeOut++;
                }

                if (status == I2C1_MESSAGE_COMPLETE)
                    break;

                // if status is  I2C1_MESSAGE_ADDRESS_NO_ACK,
                //               or I2C1_DATA_NO_ACK,
                // The device may be busy and needs more time for the last
                // write so we can retry writing the data, this is why we
                // use a while loop here

                // check for max retry and skip this byte
                if (retryTimeOut == VL53L0X_RETRY_MAX)
                    break;
                else
                    retryTimeOut++;
            }
        }

        // exit if the last transaction failed
        if (status == I2C1_MESSAGE_FAIL) {
            return (0);
            break;
        }

        pD++;
        address++;

    }
    return (1);

}