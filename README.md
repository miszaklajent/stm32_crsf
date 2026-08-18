# STM32 CRSF (Crossfire / ExpressLRS) Driver

A lightweight, non-blocking STM32 HAL driver for receiving CRSF RC channel data and sending CRSF telemetry (GPS, Battery, Link Statistics) to ExpressLRS or TBS Crossfire receivers via UART DMA.

---

## 🌟 Features

- **Bidirectional Communication**: Supports single-wire Half-Duplex (standard CRSF) or Full-Duplex.
- **DMA + Idle Line Detection**: Non-blocking packet reception using `HAL_UARTEx_ReceiveToIdle_DMA` with low CPU load.
- **RC Channels Parsing**: Extracts 16 packed 11-bit RC channels (172..1811 range, corresponding to 988..2012 µs).
- **Telemetry Transmission**:
  - GPS Telemetry (Coordinates, Groundspeed, Heading, Altitude, Satellites)
  - Battery Telemetry (Voltage, Current, Capacity, Remaining %)
  - Link Quality Statistics
- **Modular CMake Integration**: Easy integration into any STM32 CMake project or as a Git submodule.

---

# What you need to start:

1. In your vsCode or whatever, open terminal and type
   ```
   cd Drivers
   git clone https://github.com/miszaklajent/stm32_crsf.git
   ```
2. Go to your `CMakeLists.txt` in the directory of your folder
3. After 
    ``` cmake
    # Add STM32CubeMX generated sources
    add_subdirectory(cmake/stm32cubemx)
    ```
    Add
    ``` cmake
    # Add CRSF driver library
	add_subdirectory(Drivers/stm32_crsf)
    ```
4. Add `stm32_crsf` to `target_link_libraries(${CMAKE_PROJECT_NAME}`, it should look like this:
   ```cmake
    # Add linked libraries
    target_link_libraries(${CMAKE_PROJECT_NAME}
      stm32cubemx
      stm32_crsf
  
     # Add user defined libraries
    )
   ```
5. Add 
    ```c
    #include "crsf.h”
    ```
	to the beginning of the file where you want to use this library (`main.c` for example)
6. Your code should be able to compile now.
---

## 🚀 Getting Started

### 1. STM32CubeMX Setup

1. **UART Peripheral (e.g. USART2)**:
   - **Mode**: `Single Wire (Half-Duplex)` (for single pin) or `Asynchronous` (for separate TX/RX pins).
   - **Baud Rate**: `420000` or `400000` Baud (match your ELRS receiver).
   - **Word Length**: `8 Bits`, **Parity**: `None`, **Stop Bits**: `1`.
2. **DMA Configuration**:
   - `USART_RX`: DMA Stream/Channel in `Normal` mode.
   - `USART_TX`: DMA Stream/Channel in `Normal` mode.
3. **NVIC Interrupts**:
   - Enable **UART global interrupt** and **DMA stream/channel interrupts**.

---

### 2. Integration with CMake

In your main `CMakeLists.txt`:

```cmake
# Add the CRSF driver directory
add_subdirectory(Drivers/crsf)

# Link crsf library to your executable
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    crsf
)
```

---

### 3. Usage Example

#### Initialization

```c
#include "crsf.h"

// In your main() before the while loop:
crsf_init(&huart2);
```

#### Receiving RC Channels

```c
volatile uint16_t crsf_channels[16];
volatile Packet_t Packet;

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART2)
    {
        process_crsf_packet((Packet_t *)&Packet);

        if (Packet.type == TYPE_CHANNELS)
        {
            crsf_channels_t *ch = (crsf_channels_t *)Packet.payload;
            crsf_channels[0] = ch->ch1;
            crsf_channels[1] = ch->ch2;
            crsf_channels[2] = ch->ch3;
            crsf_channels[3] = ch->ch4;
            // ... channels 5 to 16
        }
    }
}

#ifdef HALF_DUPLEX
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        crsf_re_init(); // Return to receiver mode after TX completes
    }
}
#endif
```

#### Transmitting GPS Telemetry

```c
crsf_gps_t gps_data = {
    .latitude    = 522297000,          // 52.2297000 deg (deg * 1e7)
    .longitude   = 210122000,          // 21.0122000 deg (deg * 1e7)
    .groundspeed = 150,                // 15.0 km/h (km/h * 10)
    .heading     = 18000,              // 180.00 deg (deg * 100)
    .altitude    = 1000 + 150,         // 150m (altitude in meters + 1000m offset)
    .satellites  = 12                  // Satellites count
};

// Send telemetry frame to receiver / radio (0xC8 sync address)
CRSF_send_gps_data(0xC8, &gps_data);
```

---

## 🛠️ Adding as a Git Submodule

To reuse this repository in another STM32 project:

```bash
git submodule add https://github.com/<your-github-username>/stm32-crsf.git Drivers/crsf
```

---

## 📄 License

MIT License. Feel free to use in commercial and personal projects.
