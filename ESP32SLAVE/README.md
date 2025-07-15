# ESP32MASTER - ESP32 UART Communication Project

This project demonstrates  Slave asynchronous UART communication on ESP32 with separate RX and TX tasks.

## Features

- **Asynchronous UART Communication**: Separate tasks for receiving and transmitting data
- **Watchdog Timer Protection**: Proper task scheduling to prevent WDT resets
- **Flexible Command Processing**: String-based command recognition

## Hardware Requirements

- ESP32 development board
- USB-to-UART converter (for testing)


## Pin Configuration

- **UART TX**: GPIO 4
- **UART RX**: GPIO 5  


## Recent Fixes Applied

### Watchdog Timer Issues
The project was experiencing watchdog timer resets. The following fixes were applied:

1. **Task Stack Sizes**: Increased RX task stack from 4KB to 4KB and TX task to 2KB
2. **Task Priorities**: Changed from maximum priorities to reasonable levels (5 and 4)
3. **Task Scheduling**: Added `vTaskDelay(10ms)` in RX task to prevent blocking
4. **String Matching**: Changed from exact `strcmp` to flexible `strstr` matching
5. **Watchdog Timer Feeding**: Added `esp_task_wdt_reset()` calls in both tasks
6. **Configuration**: Created `sdkconfig.defaults` to disable task watchdog timer

### Code Improvements

- Better error handling in UART initialization
- Improved memory management
- More robust string processing
- Added watchdog timer subscription and feeding

## Building and Flashing

1. **Set up ESP-IDF environment**:
   ```bash
   . $HOME/esp/esp-idf/export.sh
   ```

2. **Configure the project**:
   ```bash
   idf.py menuconfig
   ```

3. **Build the project**:
   ```bash
   idf.py build
   ```

4. **Flash to ESP32**:
   ```bash
   idf.py flash monitor
   ```

## Troubleshooting

### Watchdog Timer Resets
If you still experience WDT resets:

1. Check that `sdkconfig.defaults` is properly applied
2. Verify task stack sizes are sufficient
3. Ensure tasks are yielding properly with `vTaskDelay()`
4. Monitor serial output for any error messages

### UART Communication Issues
- Verify correct baud rate (115200)
- Check pin connections
- Ensure proper USB-to-UART driver installation

## Project Structure

```
UART_02/
├── main/
│   ├── uart_async_rxtxtasks_main.c  # Main application code
│   ├── 
│   └── CMakeLists.txt               # Component build configuration
├── CMakeLists.txt                   # Project build configuration
├── sdkconfig                        # Project configuration
├── sdkconfig.defaults               # Default configuration overrides
└── README.md                        # This file
```

## License

This project is based on ESP-IDF examples and is in the Public Domain (or CC0 licensed, at your option).
