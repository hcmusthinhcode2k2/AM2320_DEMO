/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.171.4
        Device            :  dsPIC33CK256MP508
    The generated drivers are tested against the following:
        Compiler          :  XC16 v2.10
        MPLAB 	          :  MPLAB X v6.05
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/uart1.h"
#include "mcc_generated_files/delay.h"
#ifndef FCY
#define FCY (_XTAL_FREQ/2)
#endif
#include <libpic30.h>
/*
                         Main application
 */
#define AM2320_I2C_ADDR 0x5C
#define AM2320_CMD_READ 0x03
#define AM2320_REG_HUMIDITY_H 0x00
#define AM2320_REG_TEMPERATURE_H 0x02
#define AM2320_BUFFER_SIZE 8

uint16_t crc16(const uint8_t *data, uint16_t size) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

bool readAM2320(float *temperature, float *humidity) {
    uint8_t cmd[3] = {AM2320_CMD_READ, AM2320_REG_HUMIDITY_H, 4};
    uint8_t data[AM2320_BUFFER_SIZE];

    // Wake up sensor
    i2c_write1ByteRegister(AM2320_I2C_ADDR, 0x00, 0x00);
    __delay_us(200); // Wait at least 1.5ms

    // Send read command
    i2c_writeNBytes(AM2320_I2C_ADDR, cmd, 3);
    __delay_us(200); // Wait at least 1.5ms

    // Read data
//    if (!i2c_readDataBlock(AM2320_I2C_ADDR, 0x00, data, AM2320_BUFFER_SIZE)) {
//        return false;
//    }

    // Check CRC
    uint16_t receivedCrc = ((uint16_t)data[7] << 8) | data[6];
    if (crc16(data, 6) != receivedCrc) {
        return false;
    }

    // Convert data to temperature and humidity
    *humidity = ((data[2] << 8) | data[3]) / 10.0;
    *temperature = ((data[4] & 0x7F) << 8 | data[5]) / 10.0;
    if (data[4] & 0x80) { // negative temperature
        *temperature = -*temperature;
    }
    return true;
}

int main(void) {
    // Initialize the device
    SYSTEM_Initialize();
    uint16_t temperature, humidity;
    while (1) {
        if (readAM2320(&temperature, &humidity)) {
            // Use temperature and humidity values
            printf("Temperature: %.2f°C, Humidity: %.2d%%\r\n", temperature, humidity);
           __delay_ms(1000); // Delay to prevent continuous polling
        }

    }

    return 1;
}
