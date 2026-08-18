#pragma once

#include "main.h"
#include <string.h>


#define FULL_DUPLEX
// #define HALF_DUPLEX

extern volatile uint8_t latest_packet[64];

typedef struct{
    uint8_t sync;
    uint8_t length;
    uint8_t type;
    uint8_t payload_length;
    uint8_t payload[64];
} Packet_t;

typedef enum
{
    TYPE_LQ = 0x14,
    TYPE_CHANNELS = 0x16,
    TYPE_BATTERY = 0x08,
    TYPE_GPS = 0x02,
    TYPE_ALTITUDE = 0x09,
    TYPE_ATTITUDE = 0x1E
} crsf_type_t;

/**
 * @brief structure for handling 16 channels of data, 11 bits each. Which channel is used depends on transmitter setting
 *
 * @return typedef struct
 */
typedef struct __attribute__((packed))
{
    unsigned ch1 : 11;
    unsigned ch2 : 11;
    unsigned ch3 : 11;
    unsigned ch4 : 11;
    unsigned ch5 : 11;
    unsigned ch6 : 11;
    unsigned ch7 : 11;
    unsigned ch8 : 11;
    unsigned ch9 : 11;
    unsigned ch10 : 11;
    unsigned ch11 : 11;
    unsigned ch12 : 11;
    unsigned ch13 : 11;
    unsigned ch14 : 11;
    unsigned ch15 : 11;
    unsigned ch16 : 11;
} crsf_channels_t;

/**
 * @brief struct for battery data telemetry
 *
 * @param voltage the voltage of the battery in 10*V (1 = 0.1V)
 * @param current the current of the battery in 10*A (1 = 0.1A)
 * @param capacity the capacity of the battery in mah
 * @param remaining the remaining percentage of the battery
 *
 */
typedef struct __attribute__((packed))
{
    unsigned voltage : 16;  // V * 10 big endian
    unsigned current : 16;  // A * 10 big endian
    unsigned capacity : 24; // mah big endian
    unsigned remaining : 8; // %
} crsf_battery_t;

/**
 * @brief struct for link quality telemetry
 *
 * @param up_rssi_ant1 Uplink RSSI Antenna 1 (dBm * -1)
 * @param up_rssi_ant2 Uplink RSSI Antenna 2 (dBm * -1)
 * @param up_link_quality Uplink Package success rate / Link quality (%)
 * @param up_snr Uplink SNR (dB)
 * @param active_antenna number of currently best antenna
 * @param rf_profile enum {4fps = 0 , 50fps, 150fps}
 * @param up_rf_power enum {0mW = 0, 10mW, 25mW, 100mW,
 *                     500mW, 1000mW, 2000mW, 250mW, 50mW}
 * @param down_rssi Downlink RSSI (dBm * -1)
 * @param down_link_quality Downlink Package success rate / Link quality (%)
 * @param down_snr Downlink SNR (dB)
 *
 * @note up link is the signal from the tx to rx
 *       (so up_link_quality would be % of packets received by the receiver),
 *       down link is the other way around
 */
typedef struct __attribute__((packed))
{
    uint8_t up_rssi_ant1;      // Uplink RSSI Antenna 1 (dBm * -1)
    uint8_t up_rssi_ant2;      // Uplink RSSI Antenna 2 (dBm * -1)
    uint8_t up_link_quality;   // Uplink Package success rate / Link quality (%)
    int8_t up_snr;             // Uplink SNR (dB)
    uint8_t active_antenna;    // number of currently best antenna
    uint8_t rf_profile;        // enum {4fps = 0 , 50fps, 150fps}
    uint8_t up_rf_power;       // enum {0mW = 0, 10mW, 25mW, 100mW,
                               // 500mW, 1000mW, 2000mW, 250mW, 50mW}
    uint8_t down_rssi;         // Downlink RSSI (dBm * -1)
    uint8_t down_link_quality; // Downlink Package success rate / Link quality (%)
    int8_t down_snr;           // Downlink SNR (dB)
} crsf_link_t;

/**
 * @brief struct for GPS data telemetry
 *
 * @param latitude int32 the latitude of the GPS in degree / 10,000,000 big endian
 * @param longitude int32 the longitude of the GPS in degree / 10,000,000 big endian
 * @param groundspeed uint16 the groundspeed of the GPS in km/h / 10 big endian
 * @param heading uint16 the heading of the GPS in degree/100 big endian
 * @param altitude uint16 the altitude of the GPS in meters, +1000m big endian
 * @param satellites uint8 the number of satellites
 *
 */
typedef struct __attribute__((packed))
{
    int32_t latitude;     // degree / 10,000,000 big endian
    int32_t longitude;    // degree / 10,000,000 big endian
    uint16_t groundspeed; // km/h / 10 big endian
    uint16_t heading;     // GPS heading, degree/100 big endian
    uint16_t altitude;    // meters, +1000m big endian
    uint8_t satellites;   // satellites
} crsf_gps_t;




void crsf_init(UART_HandleTypeDef *huart);
void crsf_re_init();
void process_crsf_packet(Packet_t *Packet);
void transmit_crsf_channels_and_receive_telemetry(uint8_t sync, crsf_channels_t *payload);
void CRSF_send_channels_data(uint8_t sync, crsf_channels_t *payload);
void CRSF_send_gps_data(uint8_t sync, const crsf_gps_t *payload);
void CRSF_send_battery_data(uint8_t sync, crsf_battery_t *payload);
