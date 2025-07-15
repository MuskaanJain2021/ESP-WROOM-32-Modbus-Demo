# ESP-WROOM-32 Modbus Demo

## Overview

This repository demonstrates Modbus RTU communication between two ESP-WROOM-32 (ESP32) modules using the ESP-IDF framework. It provides a practical example of implementing both Modbus master and slave devices, including UART configuration, Modbus frame handling, and register read operations. The project is ideal for learning, prototyping, or as a reference for industrial applications involving Modbus and ESP32.

## Features

- **Full-duplex Modbus RTU communication** over UART (RS-485 ready)
- **Master and Slave implementations** in separate folders
- **Configurable UART pins and parameters**
- **Example holding registers** for demonstration
- **Robust CRC16 calculation** for Modbus frame integrity ([CRC16 calculator](https://www.crccalc.com/?crc=123456789&method=modbus&datatype=hex&outtype=hex))
- **Detailed logging** for debugging and learning
- **LED control** (GPIO) for visual feedback

## Project Structure

```
MODBUS/
  ├── ESP32MASTER/      # Master device code
  │   └── main/
  │       ├── uart_async_rxtxtasks_main.c
  │       ├── LED.c
  │       └── LED.h
  └── ESP32SLAVE/       # Slave device code
      └── main/
          ├── uart_async_rxtxtasks_main.c
          ├── LED.c
          └── LED.h
```

## Getting Started

### Prerequisites

- ESP-IDF installed and configured ([ESP-IDF setup guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/))
- Two ESP-WROOM-32 (ESP32) modules
- UART cross-wiring (TX ↔ RX) or RS-485 transceivers for longer distances

### Build and Flash

1. **Clone this repository:**
   ```sh
   git clone https://github.com/yourusername/esp-wroom32-modbus-demo.git
   cd esp-wroom32-modbus-demo
   ```

2. **Build and flash the master:**
   ```sh
   cd ESP32MASTER
   idf.py build
   idf.py -p <PORT_OF_MASTER> flash monitor
   ```

3. **Build and flash the slave:**
   ```sh
   cd ../ESP32SLAVE
   idf.py build
   idf.py -p <PORT_OF_SLAVE> flash monitor
   ```

4. **Connect UART lines:**
   - Master TX (GPIO 17) → Slave RX (GPIO 5)
   - Master RX (GPIO 16) ← Slave TX (GPIO 4)
   - Common GND

5. **Observe logs:**  
   The master will periodically send Modbus requests; the slave will respond with register values. All communication is logged for easy debugging.

## How It Works

### Master

- Periodically sends a Modbus RTU frame to the slave, requesting register values.
- Waits for and logs the response.
- Uses UART1 (TX: GPIO 17, RX: GPIO 16).

### Slave

- Listens for Modbus requests on UART2 (TX: GPIO 4, RX: GPIO 5).
- Validates the request, reads the requested registers, and sends a properly formatted Modbus response.
- Implements CRC16 for data integrity.

---

## Real-World Example: Energy Meter Monitoring

### Scenario

Suppose you are building a small industrial automation system. You have several energy meters (or other sensors) that communicate using the Modbus RTU protocol. You want to collect their readings using an ESP32-based master and display or log the data for monitoring and analysis.

### How This Demo Applies

- **ESP32 Slave**: Simulates an energy meter or sensor device. It holds several registers representing values such as voltage, current, power, etc.
- **ESP32 Master**: Acts as a data collector (PLC, SCADA, or gateway). It periodically queries the slave for these values and processes or displays them.

### Example Use Case

1. **Setup**:  
   - Connect the ESP32 master and slave as described above (UART or RS-485).
   - Flash the provided firmware to each device.

2. **Operation**:  
   - The master sends a Modbus request to the slave, asking for the latest readings (e.g., voltage, current, power).
   - The slave responds with the current values from its registers.
   - The master logs these values, which could be further processed, displayed on a dashboard, or sent to a cloud server.

3. **Expansion**:  
   - Replace the simulated register values in the slave with real sensor readings (e.g., from ADC, I2C, or SPI sensors).
   - Connect multiple slaves (each with a unique address) to the same bus for multi-device monitoring.
   - Integrate the master with a web server or MQTT client to visualize or transmit the data remotely.

### Example Register Mapping

| Register Address | Description      | Example Value |
|------------------|------------------|--------------|
| 0x0000           | Voltage (V)      | 230          |
| 0x0001           | Current (A)      | 5            |
| 0x0002           | Power (W)        | 1150         |
| 0x0003           | Energy (kWh)     | 12           |
| 0x0004           | Frequency (Hz)   | 50           |

**Master Request:**  
_Read 3 registers starting at address 0x0000 (Voltage, Current, Power)_

**Slave Response:**  
_Returns the current values for those registers_

---

## Modbus Frame Example

- **Master Request:**  
  `[0x01, 0x03, 0x00, 0x01, 0x00, 0x03, CRC_L, CRC_H]`  
  (Read 3 registers starting at address 0x0001 from slave 0x01)

- **Slave Normal Response:**  
  `[0x01, 0x03, 0x06, <data>, CRC_L, CRC_H]`  
  (Returns 3 registers, 2 bytes each)

- **Slave Exception Response:**  
  `[0x01, 0x83, 0x02, CRC_L, CRC_H]`  
  (Exception response: function code 0x03 + 0x80 = 0x83, exception code 0x02 = Illegal Data Address)

  - The function code in the response is the original function code with the highest bit set (0x80 added).
  - The third byte is the exception code (see table below).

### Modbus Read Holding Registers Query and Response Format

#### Master Query: Read Holding Registers (Function Code 0x03)

| Byte Index | Field                | Example Value | Description                                 |
|------------|----------------------|--------------|---------------------------------------------|
| 0          | Slave Address        | 0x01         | Address of the slave device                 |
| 1          | Function Code        | 0x03         | Read Holding Registers                      |
| 2          | Start Address Hi     | 0x00         | High byte of starting register address      |
| 3          | Start Address Lo     | 0x01         | Low byte of starting register address       |
| 4          | Quantity Hi          | 0x00         | High byte of number of registers to read    |
| 5          | Quantity Lo          | 0x03         | Low byte of number of registers to read     |
| 6          | CRC Lo               | CRC_L        | CRC16 low byte                              |
| 7          | CRC Hi               | CRC_H        | CRC16 high byte                             |

**Example:** `[0x01, 0x03, 0x00, 0x01, 0x00, 0x03, CRC_L, CRC_H]`

#### Slave Normal Response

| Byte Index | Field                | Example Value | Description                                 |
|------------|----------------------|--------------|---------------------------------------------|
| 0          | Slave Address        | 0x01         | Address of the slave device                 |
| 1          | Function Code        | 0x03         | Echoed function code                        |
| 2          | Byte Count           | 0x06         | Number of data bytes (N registers * 2)      |
| 3..n       | Register Data        | ...          | Register values (2 bytes per register)      |
| n+1        | CRC Lo               | CRC_L        | CRC16 low byte                              |
| n+2        | CRC Hi               | CRC_H        | CRC16 high byte                             |

**Example:** `[0x01, 0x03, 0x06, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, CRC_L, CRC_H]`

#### Slave Exception Response

| Byte Index | Field                | Example Value | Description                                 |
|------------|----------------------|--------------|---------------------------------------------|
| 0          | Slave Address        | 0x01         | Address of the slave device                 |
| 1          | Exception Code       | 0x83         | Function code + 0x80 (e.g., 0x03 + 0x80)    |
| 2          | Exception Type       | 0x02         | Exception code (see table above)            |
| 3          | CRC Lo               | CRC_L        | CRC16 low byte                              |
| 4          | CRC Hi               | CRC_H        | CRC16 high byte                             |

**Example:** `[0x01, 0x83, 0x02, CRC_L, CRC_H]`

### Modbus Function Codes

| Function Code | Description              |
|--------------|--------------------------|
| 0x01         | Read Coils               |
| 0x02         | Read Discrete Inputs     |
| 0x03         | Read Holding Registers   |
| 0x04         | Read Input Registers     |
| 0x05         | Write Single Coil        |
| 0x06         | Write Single Register    |
| 0x10         | Write Multiple Registers |

### Modbus Exception Codes

| Exception Code | Name                   | Meaning                                 |
|---------------|------------------------|-----------------------------------------|
| 0x01          | Illegal Function       | Function code not supported             |
| 0x02          | Illegal Data Address   | Address not available in slave          |
| 0x03          | Illegal Data Value     | Value in request is invalid             |
| 0x04          | Slave Device Failure   | Unrecoverable error in slave            |

- More details: [Modbus Exception Codes Reference](https://www.simplymodbus.ca/exceptions.htm)

### CRC16 Calculation

- All Modbus RTU frames end with a 2-byte CRC16 for error checking.
- Use this online tool to calculate/check CRC values: [CRC16 Calculator for Modbus](https://www.crccalc.com/?crc=123456789&method=modbus&datatype=hex&outtype=hex)

---

## Code Documentation

### Master (`ESP32MASTER/main/uart_async_rxtxtasks_main.c`)

#### `void master_task(void *arg)`
**Description:**  
Main FreeRTOS task for the Modbus master. Sends a Modbus request frame to the slave, waits for a response, and logs the result. Runs in a loop for continuous demonstration.

**Problem Statement:**  
How can an ESP32 act as a Modbus RTU master, sending requests and handling responses over UART?

#### `void app_main(void)`
**Description:**  
ESP-IDF entry point. Configures UART, installs the driver, sets up pins, and starts the master task.

---

### Slave (`ESP32SLAVE/main/uart_async_rxtxtasks_main.c`)

#### `uint16_t modbus_crc16(const uint8_t *buf, int len)`
**Description:**  
Calculates the Modbus CRC16 checksum for a given data buffer.

**Problem Statement:**  
How to ensure data integrity in Modbus RTU frames using CRC16?

#### `int responseHandle(const uint8_t *data, int len, uint8_t *response)`
**Description:**  
Parses a Modbus request, validates it, reads the requested registers, builds a response frame (including CRC), and handles exceptions. If the request is invalid (e.g., out-of-range address), it returns an exception response as per Modbus protocol.

**Problem Statement:**  
How to process and respond to Modbus RTU requests on an ESP32 slave, including exception handling?

#### `void slave_task(void *arg)`
**Description:**  
Main FreeRTOS task for the Modbus slave. Waits for incoming requests, processes them, and sends responses.

#### `void app_main(void)`
**Description:**  
ESP-IDF entry point. Configures UART, installs the driver, sets up pins, and starts the slave task.

---

### LED Control (`LED.c` / `LED.h` in both master and slave)

#### `void ledOnOFF(uint8_t state)`
**Description:**  
Sets the LED GPIO pin to the specified state (on/off).

#### `void configure_led(void)`
**Description:**  
Configures the LED GPIO pin as output.

---

## Customization

- **UART pins and baud rate** can be changed in the source files.
- **Register values** can be modified in the `MBREGISTERS` array in the slave code.
- **LED pin** can be changed in `LED.h`.

---

## License

This project is open-source and available under the MIT License.

---

## Acknowledgements

- [Espressif ESP-IDF](https://github.com/espressif/esp-idf)
- [Modbus Protocol Specification](https://modbus.org/)
- [Simply Modbus Exception Codes](https://www.simplymodbus.ca/exceptions.htm)
- [CRC16 Calculator for Modbus](https://www.crccalc.com/?crc=123456789&method=modbus&datatype=hex&outtype=hex) 