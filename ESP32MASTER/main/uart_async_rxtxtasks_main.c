#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

#define MASTER_UART UART_NUM_1
#define TXD_PIN (17)
#define RXD_PIN (16)
#define BUF_SIZE (256)

static const char *TAG = "MODBUS_MASTER";

void master_task(void *arg)
{
    vTaskDelay(pdMS_TO_TICKS(2000)); // wait for slave to start

    uint8_t request[] = {
        0x01,       // slave address
        0x03,       // function code or read register command
        0x00, 0x01, // starting register
        0x00, 0x03, // quantity of registers (N * data size) here data size is 2 bytes here 4 is no of bytes it needs to process /read  from 0x00 to 0x04 address
        0xC5, 0xCD  // dummy CRC
    }; // crc value will be calculated later and compared with entire modbus frame if matched then data will be processed and sent over uart/rs485 physical bus

    /*note-
     The hardware FIFO:

    Exists inside the ESP32 UART peripheral.

    Holds a small number of bytes (e.g. 128 bytes in ESP32).

    Data sits here temporarily before being moved to software.

    The software ring buffer:

    Managed by the UART driver in RAM.

    Holds data the driver copies out of the hardware FIFO via interrupts.

    So the hardware FIFO and software ring buffer are separate.
    The hardware FIFO is where data is received from the physical UART bus, while the software ring buffer is where the driver stores
    that data for processing.



    */

    //here if we want to send data over uart/rs485 physical bus we need to use uart_write_bytes function
    //which will write the data to the hardware FIFO and then the driver will copy it to
    //the software ring buffer for processing.
    //The driver will then send the data over the physical bus using the hardware FIFO.
    //The driver will also handle the CRC calculation and verification for the modbus frame.
    //The request is a modbus frame that will be sent over the uart/rs485 physical bus.
    //while (1) we want to send the request continuously
    //and wait for the response from the slave device.
    //The response will be received in the software ring buffer and then processed by the driver.
    //it depends on user how many times he wants to send the request
    //and how many times he wants to receive the response.
while(1){    uart_write_bytes(MASTER_UART, request, sizeof(request));
    ESP_LOGI(TAG, "Master sent request:");
    ESP_LOG_BUFFER_HEXDUMP(TAG, request, sizeof(request), ESP_LOG_INFO);

    uint8_t response[BUF_SIZE] = {0};
    int len = uart_read_bytes(MASTER_UART, response, sizeof(response), pdMS_TO_TICKS(5000));
    if (len > 0)
    {
        ESP_LOGI(TAG, "Master received response (%d bytes):", len);
        ESP_LOG_BUFFER_HEXDUMP(TAG, response, len, ESP_LOG_INFO);
    }
    else
    {
        ESP_LOGW(TAG, "Master received no response");
    }
}
    //vTaskDelete(NULL);
    

/*
    Syntax:
        void vTaskDelete(TaskHandle_t xTaskToDelete);

    Deletes a task in FreeRTOS.

    - The task to be deleted is specified by passing its task handle as the parameter.
    - If the handle is NULL, the calling task (the currently running task) will be deleted.

    In this case, the calling task is master_task. It will be deleted after sending the request
    and receiving the response from the slave.

    Important notes:
    - If the task being deleted is the currently running task, it will be deleted immediately,
      and the function does not return to the caller.
    - This function is typically used to delete tasks that are no longer needed,
      such as when a task has completed its work or during application shutdown.
    - To delete the currently running task, use:
          vTaskDelete(NULL);

    This function does not return any value.
*/

}

void app_main(void)
{
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_driver_install(MASTER_UART, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(MASTER_UART, &uart_config);
    uart_set_pin(MASTER_UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    xTaskCreate(master_task, "master_task", 4096, NULL, 10, NULL);
}
