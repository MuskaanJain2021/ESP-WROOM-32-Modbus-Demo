#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

#define SLAVE_UART UART_NUM_2
#define TXD_PIN    (4)
#define RXD_PIN    (5)
#define BUF_SIZE   (256)

static const char *TAG = "MODBUS_SLAVE";


uint16_t MBRESGESTERS[] = {0x0001, 0x0002, 0x0003, 0x0004, 0x0005}; //modbus registers as in voltage parameter,current, power, energy, frequency, etc.
//index treated as register address
//inside MBRESGESTERS array, each element is a 16-bit register value which can be read by the master
//Master Query // 01(slaveID) 03(function code) 00(start address)Hi 01(start address)  02 (number of registers/uint16 to be read ) CRC16
// Slave Response // 01(slaveID) 03(function code) 02(uint8(lenth of data in bytes)) ( Response data) byte count CRC16

//for crc check this calculator https://www.crccalc.com/?crc=123456789&method=&datatype=hex&outtype=hex
//here 2 bytes are used for crc16, so the response will be 5 bytes long



// Example holding registers:
// index treated as register address
uint16_t MBREGISTERS[] = {
    0x0001, // address 0
    0x0002, // address 1
    0x0003, // address 2
    0x0004, // address 3
    0x0005  // address 4
};

// Function to calculate Modbus CRC16
uint16_t modbus_crc16(const uint8_t *buf, int len)
{
    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// Function to handle Modbus request and build response
int responseHandle(const uint8_t *data, int len, uint8_t *response)
{
    if (len < 8) return 0; // frame too short

    uint8_t slave_id = data[0];
    uint8_t function = data[1];

    if (slave_id != 0x01) {
        // not for this slave
        return 0;
    }

    if (function == 0x03) {
        // Read Holding Registers
        uint16_t start_address = (data[2] << 8) | data[3];
        uint16_t quantity = (data[4] << 8) | data[5];

        // Validate range
        if (start_address + quantity > sizeof(MBREGISTERS)/sizeof(uint16_t)) {
            // Illegal data address error response
            response[0] = slave_id;
            response[1] = function | 0x80;
            // Illegal data address exception check for error codes hex value to be generated in response to the read/ write operation
            // https://www.simplymodbus.ca/exceptions.htm#:~:text=This%20coil%20has%20not%20been%20defined%20in%20the,and%20was%20not%20implemented%20in%20the%20unit%20selected.
            response[2] = 0x02; // exception code: illegal data address

            uint16_t crc = modbus_crc16(response, 3);
            response[3] = crc & 0xFF;
            response[4] = (crc >> 8) & 0xFF;
            return 5;
        }

        // Build normal response
        response[0] = slave_id;
        response[1] = function;
        response[2] = quantity * 2; // byte count

        int idx = 3;
        for (int i = 0; i < quantity; i++) {
            uint16_t reg_val = MBREGISTERS[start_address + i];
            response[idx++] = (reg_val >> 8) & 0xFF;
            response[idx++] = reg_val & 0xFF;
        }

        uint16_t crc = modbus_crc16(response, idx);
        response[idx++] = crc & 0xFF;
        response[idx++] = (crc >> 8) & 0xFF;

        return idx;
    }

    // Unsupported function code
    response[0] = slave_id;
    response[1] = function | 0x80;
    response[2] = 0x01; // Illegal function

    uint16_t crc = modbus_crc16(response, 3);
    response[3] = crc & 0xFF;
    response[4] = (crc >> 8) & 0xFF;
    return 5;
}

void slave_task(void *arg)
{
    uint8_t data[BUF_SIZE];

    while (1) {
        int len = uart_read_bytes(SLAVE_UART, data, sizeof(data), pdMS_TO_TICKS(1000));
        if (len > 0) {
            ESP_LOGI(TAG, "Slave received %d bytes:", len);
            ESP_LOG_BUFFER_HEXDUMP(TAG, data, len, ESP_LOG_INFO);

            uint8_t response[BUF_SIZE];
            int resp_len = responseHandle(data, len, response);

            if (resp_len > 0) {
                uart_write_bytes(SLAVE_UART, response, resp_len);
                ESP_LOGI(TAG, "Slave sent response (%d bytes):", resp_len);
                ESP_LOG_BUFFER_HEXDUMP(TAG, response, resp_len, ESP_LOG_INFO);
            }
        }
    }
}

void app_main(void)
{
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(SLAVE_UART, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(SLAVE_UART, &uart_config);
    uart_set_pin(SLAVE_UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    xTaskCreate(slave_task, "slave_task", 4096, NULL, 10, NULL);
}
